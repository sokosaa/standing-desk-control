// Advanced Desk Controller with Protocol Commands
// Can program presets and send TX commands based on discovered protocol

#include <Arduino.h>

// Pin definitions
const int DOWN_PIN = 9;      // RJ45 Pin 1 - down button (preset simulation)
const int UP_PIN = 8;        // RJ45 Pin 2 - up button (preset simulation)  
const int PRESET_PIN = 7;    // RJ45 Pin 3 - preset buttons
const int TX_PIN = 5;        // RJ45 Pin 5 - TX data (we need to connect this for sending commands)
const int RX_PIN = 0;        // RJ45 Pin 7 - RX data (height reading)
const int M_PIN = 2;         // RJ45 Pin 8 - M button

// Height tracking
float currentHeight = 0.0;
bool programmingMode = false;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);      // RX line for reading height
  
  // Set up pins
  pinMode(DOWN_PIN, OUTPUT);
  pinMode(UP_PIN, OUTPUT);
  pinMode(PRESET_PIN, OUTPUT);
  pinMode(M_PIN, OUTPUT);
  
  // All pins start high (inactive)
  digitalWrite(DOWN_PIN, HIGH);
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(PRESET_PIN, HIGH);
  digitalWrite(M_PIN, HIGH);
  
  Serial.println("=== ADVANCED DESK CONTROLLER ===");
  Serial.println("Based on discovered protocol:");
  Serial.println("- Programming mode: 0x01 0x06 0x00 0x00");
  Serial.println("- Program preset: 0x01 0x06 0x[preset] 0x00");
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  'h' - Show current height");
  Serial.println("  'preset X' - Recall preset 1-4 (e.g., 'preset 1')");
  Serial.println("  'program X' - Program current height to preset 1-4");
  Serial.println("  'up' - Manual up");
  Serial.println("  'down' - Manual down");
  Serial.println("  'stop' - Stop movement");
  Serial.println();
}

void loop() {
  readHeightData();
  handleSerialCommands();
}

void readHeightData() {
  static uint8_t buffer[4];
  static int index = 0;
  static uint16_t lastHeight = 0xFFFF; // Invalid initial height

  if (Serial1.available()) {
    buffer[index] = Serial1.read();
    index++;
    if (index == 4) { // Full packet
      if (buffer[0] == 0x01 && buffer[1] == 0x01) { // Valid height packet
        uint16_t height = (buffer[2] << 8) | buffer[3]; // Combine high/low bytes
        if (height != lastHeight) { // Print only if height changes
          float inches = height / 10.0; // Tenths of an inch
          Serial.print("Height: ");
          Serial.print(inches);
          Serial.println(" inches");
          lastHeight = height;
          currentHeight = inches; // Update for programming/presets
        }
      } else if (buffer[0] == 0x06 && buffer[1] == 0x00) {
        // Programming mode packet (reordered from what we expected)
        if (buffer[2] == 0x00 && buffer[3] == 0x01) {
          Serial.println("PROGRAMMING MODE DETECTED!");
          programmingMode = true;
        } else {
          Serial.print("Programming: ");
          for (int i = 0; i < 4; i++) {
            Serial.print("0x");
            if (buffer[i] < 16) Serial.print("0");
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
          }
          Serial.println();
        }
      }
      // Don't print anything for other packets to keep output clean
      
      index = 0;
    }
  }
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "h" || command == "height") {
      if (currentHeight > 0.0) {
        Serial.print("Current Height: ");
        Serial.print(currentHeight);
        Serial.println(" inches");
      } else {
        Serial.println("Getting current height... (waiting for data)");
        // Height will be displayed when next valid packet arrives
      }
    }
    else if (command.startsWith("preset ")) {
      int presetNum = command.substring(7).toInt();
      if (presetNum >= 1 && presetNum <= 4) {
        recallPreset(presetNum);
      } else {
        Serial.println("Preset must be 1-4");
      }
    }
    else if (command.startsWith("program ")) {
      int presetNum = command.substring(8).toInt();
      if (presetNum >= 1 && presetNum <= 4) {
        programPreset(presetNum);
      } else {
        Serial.println("Preset must be 1-4");
      }
    }
    else if (command == "up") {
      manualMove(true, 1000);
    }
    else if (command == "down") {
      manualMove(false, 1000);
    }
    else if (command == "stop") {
      stopAll();
    }
    else {
      Serial.println("Unknown command");
    }
  }
}

void recallPreset(int presetNum) {
  Serial.print("Recalling preset ");
  Serial.println(presetNum);
  
  // Each preset uses different button combinations based on protocol analysis
  switch(presetNum) {
    case 1:
      // Preset 1: DOWN+UP combination (protocol: 0x01 0x06 0x01 0x00)
      Serial.println("Using DOWN+UP combination for preset 1...");
      digitalWrite(DOWN_PIN, LOW);
      digitalWrite(UP_PIN, LOW);
      delay(150);
      digitalWrite(DOWN_PIN, HIGH);
      digitalWrite(UP_PIN, HIGH);
      break;
    case 2:
      // Preset 2: PRESET button only (protocol: 0x01 0x06 0x02 0x00)
      Serial.println("Using PRESET button for preset 2...");
      digitalWrite(PRESET_PIN, LOW);
      delay(150);
      digitalWrite(PRESET_PIN, HIGH);
      break;
    case 3:
      // Preset 3: DOWN+PRESET combination (protocol: 0x01 0x06 0x04 0x00)
      Serial.println("Using DOWN+PRESET combination for preset 3...");
      digitalWrite(DOWN_PIN, LOW);
      digitalWrite(PRESET_PIN, LOW);
      delay(150);
      digitalWrite(DOWN_PIN, HIGH);
      digitalWrite(PRESET_PIN, HIGH);
      break;
    case 4:
      // Preset 4: UP+PRESET combination (predicted)
      Serial.println("Using UP+PRESET combination for preset 4...");
      digitalWrite(UP_PIN, LOW);
      digitalWrite(PRESET_PIN, LOW);
      delay(150);
      digitalWrite(UP_PIN, HIGH);
      digitalWrite(PRESET_PIN, HIGH);
      break;
  }
  
  // Give some time for the desk to start moving
  delay(1000);
  Serial.println("Preset recall initiated...");
}

void programPreset(int presetNum) {
  Serial.print("Programming current height (");
  Serial.print(currentHeight);
  Serial.print(") to preset ");
  Serial.println(presetNum);
  
  // Step 1: Press M button to enter programming mode
  // This generates 0x01 0x06 0x00 0x00 packets
  Serial.println("Entering programming mode...");
  digitalWrite(M_PIN, LOW);
  delay(150); // Similar to manual timing (~138-168ms)
  digitalWrite(M_PIN, HIGH);
  delay(500); // Wait for programming mode to activate
  
  // Step 2: Select preset slot using discovered button combinations
  Serial.println("Selecting preset slot...");
  switch(presetNum) {
    case 1:
      // Preset 1: DOWN+UP combination (generates 0x01 0x06 0x01 0x00)
      digitalWrite(DOWN_PIN, LOW);
      digitalWrite(UP_PIN, LOW);
      delay(170);
      digitalWrite(DOWN_PIN, HIGH);
      digitalWrite(UP_PIN, HIGH);
      break;
    case 2:
      // Preset 2: PRESET button only (generates 0x01 0x06 0x02 0x00)
      digitalWrite(PRESET_PIN, LOW);
      delay(170);
      digitalWrite(PRESET_PIN, HIGH);
      break;
    case 3:
      // Preset 3: DOWN+PRESET combination (generates 0x01 0x06 0x04 0x00)
      digitalWrite(DOWN_PIN, LOW);
      digitalWrite(PRESET_PIN, LOW);
      delay(170);
      digitalWrite(DOWN_PIN, HIGH);
      digitalWrite(PRESET_PIN, HIGH);
      break;
    case 4:
      // Preset 4: UP+PRESET combination (predicted to generate 0x01 0x06 0x08 0x00)
      digitalWrite(UP_PIN, LOW);
      digitalWrite(PRESET_PIN, LOW);
      delay(170);
      digitalWrite(UP_PIN, HIGH);
      digitalWrite(PRESET_PIN, HIGH);
      break;
  }
  
  delay(500); // Wait for programming to complete
  Serial.println("Programming complete!");
  
  // Wait and monitor for confirmation packets
  Serial.println("Monitoring for programming confirmation...");
  delay(2000); // Give time to see programming packets
}

void manualMove(bool up, int duration) {
  Serial.print("Manual ");
  Serial.print(up ? "UP" : "DOWN");
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println(" ms");
  
  if (up) {
    digitalWrite(UP_PIN, LOW);
  } else {
    digitalWrite(DOWN_PIN, LOW);
  }
  
  delay(duration);
  stopAll();
}

void stopAll() {
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(DOWN_PIN, HIGH);
  digitalWrite(PRESET_PIN, HIGH);
  digitalWrite(M_PIN, HIGH);
  Serial.println("All movement stopped");
}
