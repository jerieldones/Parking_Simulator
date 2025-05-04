/**
 * Parking Spot Controller (Slave) with RFID, FSRs, Ultrasonic Sensor, and OLED Display
 *
 * This sketch operates as a secondary node in a parking system:
 *  - Reads an RFID tag and opens/closes a servo-driven gate.
 *  - Uses an ultrasonic sensor to detect vehicle passage and close the gate.
 *  - Monitors 3 parking spots using force-sensitive resistors (FSRs).
 *  - Displays spot availability and statuses on an SSD1306 OLED display.
 *
 * This controller does not communicate with the master Arduino via UART,
 * but can be adapted for expansion.
 *
 * Hardware Connections:
 *  - MFRC522 RFID      : SS → pin 10, RST → pin 9, SDA/SCK/MOSI/MISO → SPI
 *  - Servo Gate        : Control → pin 3
 *  - Ultrasonic Sensor : TRIG → pin 7, ECHO → pin 8
 *  - FSR Pressure Pads : Analog inputs A0, A1, A2
 *  - SSD1306 OLED      : I2C SDA → A4, SCL → A5 (or 20/21 on Mega)
 *
 * Author: Jeriel Dones Aguayo, Abdiel Gomez Alverio
 * Date: April 2025
 */

 #include <Wire.h>                    // I2C communication (OLED)
 #include <Adafruit_GFX.h>           // Graphics library for OLED
 #include <Adafruit_SSD1306.h>       // OLED driver library
 #include <SPI.h>                    // SPI communication (RFID)
 #include <MFRC522.h>                // RFID reader library
 #include <Servo.h>                  // Servo motor control
 
 // —— Pin Definitions —— 
 #define SS_PIN         10           // RFID SS pin
 #define RST_PIN         9           // RFID reset pin
 #define SERVO_PIN       3           // Servo signal pin
 #define TRIG_PIN        7           // Ultrasonic trigger pin
 #define ECHO_PIN        8           // Ultrasonic echo pin
 
 #define FSR1_PIN        A0          // Parking spot 1 FSR
 #define FSR2_PIN        A1          // Parking spot 2 FSR
 #define FSR3_PIN        A2          // Parking spot 3 FSR
 #define FSR_THRESHOLD   500         // Threshold for FSR1 (custom thresholds for FSR2/3)
 
 #define SCREEN_WIDTH    128         // OLED width (pixels)
 #define SCREEN_HEIGHT    64         // OLED height (pixels)
 #define OLED_RESET      -1          // OLED reset pin (not used)
 
 // —— Global Objects —— 
 Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 MFRC522 rfid(SS_PIN, RST_PIN);      // RFID reader object
 Servo gateServo;                    // Servo motor object
 
 // —— RFID Authorization —— 
 byte authorizedUID[4] = { 0x03, 0x0C, 0x49, 0x16 }; // Pre-defined authorized UID
 
 // —— Parking Spot Management —— 
 const int totalSpots = 3;
 int availableSpots = totalSpots;
 bool gateOpen = false;
 
 void setup() {
   Serial.begin(9600);               // Initialize serial monitor
 
   // RFID setup
   SPI.begin();                      // Start SPI bus
   rfid.PCD_Init();                  // Init RFID module
   Serial.println(F("Scan your RFID tag..."));
 
   // Servo setup
   gateServo.attach(SERVO_PIN);
   gateServo.write(90);              // Default to closed position
 
   // Ultrasonic sensor setup
   pinMode(TRIG_PIN, OUTPUT);
   pinMode(ECHO_PIN, INPUT);
 
   // OLED initialization
   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
     Serial.println(F("SSD1306 allocation failed"));
     while (true);                   // Stop if OLED fails
   }
   display.display();                // Show splash screen
   delay(2000);
   display.clearDisplay();
 }
 
 void loop() {
   // ——— 1) Read FSRs and Count Available Spots ——— 
   int fsr1 = analogRead(FSR1_PIN);
   int fsr2 = analogRead(FSR2_PIN);
   int fsr3 = analogRead(FSR3_PIN);
 
   bool spot1Free = fsr1 < FSR_THRESHOLD;
   bool spot2Free = fsr2 < 270;     // Custom threshold for FSR2
   bool spot3Free = fsr3 < 400;     // Custom threshold for FSR3
 
   availableSpots = 0;
   if (spot1Free) availableSpots++;
   if (spot2Free) availableSpots++;
   if (spot3Free) availableSpots++;
 
   Serial.print("FSR1: "); Serial.print(fsr1);
   Serial.print(" | FSR2: "); Serial.print(fsr2);
   Serial.print(" | FSR3: "); Serial.print(fsr3);
   Serial.print(" || Available: "); Serial.println(availableSpots);
 
   // ——— 2) RFID Authentication ——— 
   if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
     if (isAuthorized(rfid.uid.uidByte)) {
       Serial.println(F("✅ Access Granted – Opening Gate"));
       gateServo.write(0);           // Open gate
       gateOpen = true;
       delay(2000);                  // Wait for vehicle to enter
     } else {
       Serial.println(F("❌ Access Denied – UID not recognized"));
     }
     rfid.PICC_HaltA();              // Stop reading the current tag
   }
 
   // ——— 3) Auto-Close Gate After Vehicle Entry ——— 
   if (gateOpen) {
     float distance = readDistanceCM();
     Serial.print(F("Distance: "));
     Serial.print(distance);
     Serial.println(F(" cm"));
 
     if (distance <= 12.0) {
       Serial.println(F("Vehicle passed – closing gate"));
       delay(2500);                  // Allow vehicle to fully pass
       gateServo.write(90);          // Close gate
       gateOpen = false;
     }
   }
 
   // ——— 4) OLED Display ——— 
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
 
   display.display();                // Push new display content
 
   delay(500);                       // Short delay before next loop
 }
 
 /**
  * Checks whether a scanned UID is authorized.
  */
 bool isAuthorized(byte *uid) {
   for (byte i = 0; i < 4; i++) {
     if (uid[i] != authorizedUID[i]) return false;
   }
   return true;
 }
 
 /**
  * Reads and returns distance (in cm) from ultrasonic sensor.
  */
 float readDistanceCM() {
   digitalWrite(TRIG_PIN, LOW);
   delayMicroseconds(2);
   digitalWrite(TRIG_PIN, HIGH);
   delayMicroseconds(10);
   digitalWrite(TRIG_PIN, LOW);
 
   long duration = pulseIn(ECHO_PIN, HIGH);
   return duration * 0.034f / 2.0f;
 } 