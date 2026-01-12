# Smart Parking Spot Controller (IoT System) 
<p align = "center"> <img width="718" height="668" alt="image" src="https://github.com/user-attachments/assets/899eb0fe-4814-4cd3-9cb1-135a9745070c" /> </p>

## Overview
The Smart Parking Spot Controller is a standalone IoT system designed to automate vehicle access control, parking occupancy monitoring, local visualization, and cloud-based reporting using a single microcontroller. The system authenticates vehicles using RFID, operates a servo-controlled gate, detects vehicle passage with an ultrasonic sensor, monitors three parking spaces via force-sensitive resistors (FSRs), displays real-time status on an OLED screen, and publishes parking availability to the Blynk Cloud platform over WiFi.

## Features: 
- RFID-based access authorization
- Automated servo-driven gate control
- Ultrasonic-based vehicle passage detection
- Real-time parking occupancy monitoring (3 spots)
- OLED local status display
- WiFi cloud connectivity using WiFiS3
- Live parking availability updates via Blynk

## System Design: 
This system functions as an all-in-one smart parking controller, eliminating the need for a master/slave architecture. The microcontroller is responsible for: 
- Sensor data acquisition
- Access control logic
- Actuator control
- Local user interface
- Cloud communication

The architecture is modular and scalable, allowing additional sensors, gates, or analytics to be integrated in future revisions. 

## Hardware Components 
Component & Function: 
- WiFi-enabled Arduino -> Central controller
- MFRC522 RFID Reader -> User authentication
- Servo Motor -> Gate actuation
- Ultrasonic Sensor (HC-SR04) -> Vehicle detection
- 3× FSR Sensors -> Parking spot occupancy
- SSD1306 OLED (128×64) -> Local display
- WiFi (WiFiS3) -> Cloud connectivity

## Hardware Connections 
RFID (MFRC522)
- SS → Pin 10
- RST → Pin 9
- SPI → MOSI / MISO / SCK

Sensors & Actuators 
- Servo Motor → Pin 3
- Ultrasonic TRIG → Pin 7
- Ultrasonic ECHO → Pin 8 Parking Sensors
- FSR 1 → A0
- FSR 2 → A1
- FSR 3 → A2 OLED Display (I²C)
- SDA → A4
- SCL → A5

## Cloud & App Integration (Blynk)
- Platform: Blynk Cloud
- Template ID: TMPL2JrlKDUrB
- Virtual Pin: V0
- Published Data: Number of available parking spaces

The Blynk dashboard updates automatically whenever a parking space becomes occupied or available. 

## OLED User Interface 
The OLED display provides: 
- Entry prompt (Insert ID)
- Current available parking count
- Individual spot indicators
- Visual capacity bar indicating system load

## RFID Access Control 
- Only authorized RFID cards are granted access
- Unauthorized attempts are rejected
- Authorization is UID-based and easily extendable

## System Operation Flow 
1. Read FSR sensors and determine occupancy
2. Update OLED display and availability metrics
3. Wait for RFID authentication
4. Open gate upon valid authorization
5. Detect vehicle passage with ultrasonic sensor
6. Automatically close gate
7. Sync parking availability with Blynk

## Software & Libraries 
- WiFiS3
- BlynkSimpleWifi
- MFRC522
- Servo
- Adafruit_GFX
- Adafruit_SSD1306
