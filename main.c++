#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// —— Pin Definitions ——
#define SS_PIN     10
#define RST_PIN     9
#define SERVO_PIN   3
#define TRIG_PIN    7
#define ECHO_PIN    8
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET     -1

#define FSR1_PIN   A0
#define FSR2_PIN   A1
#define FSR3_PIN   A2
#define FSR_THRESHOLD 500

// —— Global Objects ——
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 rfid(SS_PIN, RST_PIN);
Servo gateServo;

// —— Authorized Access ——
byte authorizedUID[4] = { 0x03, 0x0C, 0x49, 0x16 };

// —— Parking Spot Management ——
const int totalSpots = 3;
int availableSpots = totalSpots;
bool gateOpen = false;

void setup() {
  Serial.begin(9600);

  // RFID initialization
  SPI.begin();
  rfid.PCD_Init();
  Serial.println(F("Scan your RFID tag..."));

  // Servo setup
   Serial.print("SERVO SETUP 1");
  gateServo.attach(SERVO_PIN);
   Serial.print("SERVO SETUP 2");
  gateServo.write(90); // Start closed
   Serial.print("SERVO SETUP 3");

  // Ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.display();
  delay(2000);
}

void loop() {
  // 1) Read FSRs and calculate available spots
  int fsr1 = analogRead(FSR1_PIN);
  int fsr2 = analogRead(FSR2_PIN);
  int fsr3 = analogRead(FSR3_PIN);

  bool spot1Free = fsr1 < FSR_THRESHOLD;
  bool spot2Free = fsr2 < 270;
  bool spot3Free = fsr3 < 400;

  availableSpots = 0;
  if (spot1Free) availableSpots++;
  if (spot2Free) availableSpots++;
  if (spot3Free) availableSpots++;

  Serial.print("FSR1: "); Serial.print(fsr1);
  Serial.print(" | FSR2: "); Serial.print(fsr2);
  Serial.print(" | FSR3: "); Serial.print(fsr3);
  Serial.print(" || Available: "); Serial.println(availableSpots);

  // 2) RFID Access Control
  bool gotCard = rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();
  if (gotCard) {
    if (isAuthorized(rfid.uid.uidByte)) {
      Serial.println("✅ Access Granted – Opening Gate");
      gateOpen = true;
      gateServo.write(0); // open
      delay(2000);
    } else {
      Serial.println("❌ Access Denied – UID not recognized");
    }
    rfid.PICC_HaltA();
  }

  // 3) Auto-close gate using ultrasonic
  if (gateOpen) {
    float distance = readDistanceCM();
    Serial.print(F("Distance: "));
    Serial.print(distance);
    Serial.println(F(" cm"));

    if (distance <= 12.0) {
      Serial.println(F("Vehicle passed – closing gate"));
      delay(5000);
      gateServo.write(90);
      gateOpen = false;
    }
  }

  // 4) OLED Display
  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(10, 5);
  display.println(F("Insert ID"));

  display.setCursor(10, 20);
  display.print(F("Available: "));
  display.println(availableSpots);

  display.setCursor(10, 35);
  display.print(F("S1: ")); display.print(spot1Free ? "O" : "X");
  display.print(F(" S2: ")); display.print(spot2Free ? "O" : "X");
  display.print(F(" S3: ")); display.println(spot3Free ? "O" : "X");

  int barWidth = map(availableSpots, 0, totalSpots, 0, SCREEN_WIDTH);
  display.fillRect(0, SCREEN_HEIGHT - 9, barWidth, 5, SSD1306_WHITE);

  display.display();
  delay(500);
}

// —— RFID UID check ——
bool isAuthorized(byte *uid) {
  for (byte i = 0; i < 4; i++) {
    if (uid[i] != authorizedUID[i]) return false;
  }
  return true;
}

// —— Ultrasonic Distance Calculation ——
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034f / 2.0f;
}