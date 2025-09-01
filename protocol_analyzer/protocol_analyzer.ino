// Desk Protocol Analyzer
// Monitor all communication and button states to understand the protocol

#include <Arduino.h>

// Pin definitions from your setup
const int DOWN_PIN = 9;      // RJ45 Pin 1 - down button
const int UP_PIN = 8;        // RJ45 Pin 2 - up button  
const int PRESET_PIN = 7;    // RJ45 Pin 3 - preset buttons
const int TX_PIN = 5;        // RJ45 Pin 5 - TX data (controller to desk)
const int RX_PIN = 0;        // RJ45 Pin 7 - RX data (desk to controller) 
const int M_PIN = 2;         // RJ45 Pin 8 - M button

// Note: For now we'll focus on button monitoring and RX data
// TX monitoring would need additional hardware or different approach

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);      // Monitor RX line (height data)
  
  // Set button pins as inputs
  pinMode(DOWN_PIN, INPUT);
  pinMode(UP_PIN, INPUT);
  pinMode(PRESET_PIN, INPUT);
  pinMode(M_PIN, INPUT);
  
  Serial.println("=== DESK PROTOCOL ANALYZER (R4 Compatible) ===");
  Serial.println("Monitoring:");
  Serial.println("- Button states (Down, Up, Preset, M)");
  Serial.println("- RX height data (desk to controller)");
  Serial.println("- Button press patterns and timing");
  Serial.println();
  Serial.println("Try pressing:");
  Serial.println("1. Up/Down buttons (hold vs tap)");
  Serial.println("2. Preset buttons (1,2,3,4)");
  Serial.println("3. M button");
  Serial.println("4. Different button combinations");
  Serial.println();
}

void loop() {
  static unsigned long lastButtonCheck = 0;
  static bool lastDown = HIGH, lastUp = HIGH, lastPreset = HIGH, lastM = HIGH;
  static unsigned long buttonPressStart = 0;
  static bool buttonHeld = false;
  
  // Check button states every 10ms for better responsiveness
  if (millis() - lastButtonCheck >= 10) {
    bool down = digitalRead(DOWN_PIN);
    bool up = digitalRead(UP_PIN);
    bool preset = digitalRead(PRESET_PIN);
    bool m = digitalRead(M_PIN);
    
    // Track button press timing
    bool anyPressed = (down == LOW || up == LOW || preset == LOW || m == LOW);
    if (anyPressed && !buttonHeld) {
      buttonPressStart = millis();
      buttonHeld = true;
    } else if (!anyPressed && buttonHeld) {
      unsigned long duration = millis() - buttonPressStart;
      Serial.print("Button held for: ");
      Serial.print(duration);
      Serial.println(" ms");
      buttonHeld = false;
    }
    
    // Report button state changes with timing
    if (down != lastDown) {
      Serial.print("DOWN: ");
      Serial.print(down == LOW ? "PRESSED" : "RELEASED");
      Serial.print(" at ");
      Serial.println(millis());
    }
    if (up != lastUp) {
      Serial.print("UP: ");
      Serial.print(up == LOW ? "PRESSED" : "RELEASED");
      Serial.print(" at ");
      Serial.println(millis());
    }
    if (preset != lastPreset) {
      Serial.print("PRESET: ");
      Serial.print(preset == LOW ? "PRESSED" : "RELEASED");
      Serial.print(" at ");
      Serial.println(millis());
    }
    if (m != lastM) {
      Serial.print("M BUTTON: ");
      Serial.print(m == LOW ? "PRESSED" : "RELEASED");
      Serial.print(" at ");
      Serial.println(millis());
    }
    
    lastDown = down;
    lastUp = up; 
    lastPreset = preset;
    lastM = m;
    lastButtonCheck = millis();
  }
  
  // Monitor RX line (height/status from desk)
  readHeightAndStatus();
}

void readHeightAndStatus() {
  static uint8_t buffer[4];
  static int index = 0;
  static uint16_t lastHeight = 0xFFFF;

  if (Serial1.available()) {
    buffer[index] = Serial1.read();
    index++;
    if (index == 4) { // Full packet
      // Print raw packet for analysis
      Serial.print("RX RAW: ");
      for (int i = 0; i < 4; i++) {
        Serial.print("0x");
        if (buffer[i] < 16) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }
      
      if (buffer[0] == 0x01 && buffer[1] == 0x01) { // Height packet
        uint16_t height = (buffer[2] << 8) | buffer[3];
        if (height != lastHeight && height < 1000) { // Filter noise
          float inches = height / 10.0;
          Serial.print("-> HEIGHT: ");
          Serial.print(inches);
          Serial.println(" inches");
          lastHeight = height;
        } else {
          Serial.println("-> (height packet)");
        }
      } else {
        Serial.println("-> (unknown packet)");
      }
      index = 0;
    }
  }
}
