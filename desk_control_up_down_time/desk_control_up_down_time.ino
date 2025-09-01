// Standing Desk Control
// Pin 1 (Arduino pin 9) - Down control
// Pin 2 (Arduino pin 8) - Up control

#include <Arduino.h>

const int DOWN_PIN = 9;  // RJ45 Pin 1 - goes low to move down
const int UP_PIN = 8;    // RJ45 Pin 2 - goes low to move up

void setup() {
  Serial.begin(115200);
  
  // Set pins as outputs initially high (inactive)
  pinMode(DOWN_PIN, OUTPUT);
  pinMode(UP_PIN, OUTPUT);
  digitalWrite(DOWN_PIN, HIGH);
  digitalWrite(UP_PIN, HIGH);
  
  Serial.println("Standing Desk Controller Ready");
  Serial.println("Commands:");
  Serial.println("  'u' or 'up' - Move desk up for 1 second");
  Serial.println("  'd' or 'down' - Move desk down for 1 second");
  Serial.println("  'u[time]' - Move up for specified milliseconds (e.g., 'u2000')");
  Serial.println("  'd[time]' - Move down for specified milliseconds (e.g., 'd500')");
  Serial.println("  's' or 'stop' - Emergency stop");
  Serial.println();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "u" || command == "up") {
      moveUp(1000);
    }
    else if (command == "d" || command == "down") {
      moveDown(1000);
    }
    else if (command.startsWith("u")) {
      int duration = command.substring(1).toInt();
      if (duration > 0 && duration <= 10000) { // Max 10 seconds for safety
        moveUp(duration);
      } else {
        Serial.println("Invalid duration. Use 1-10000 milliseconds.");
      }
    }
    else if (command.startsWith("d")) {
      int duration = command.substring(1).toInt();
      if (duration > 0 && duration <= 10000) { // Max 10 seconds for safety
        moveDown(duration);
      } else {
        Serial.println("Invalid duration. Use 1-10000 milliseconds.");
      }
    }
    else if (command == "s" || command == "stop") {
      emergencyStop();
    }
    else {
      Serial.println("Unknown command. Use 'u', 'd', 's', or help.");
    }
  }
}

void moveUp(int duration) {
  Serial.print("Moving UP for ");
  Serial.print(duration);
  Serial.println(" ms");
  
  digitalWrite(UP_PIN, LOW);   // Activate up movement
  delay(duration);
  digitalWrite(UP_PIN, HIGH);  // Deactivate
  
  Serial.println("UP movement complete");
}

void moveDown(int duration) {
  Serial.print("Moving DOWN for ");
  Serial.print(duration);
  Serial.println(" ms");
  
  digitalWrite(DOWN_PIN, LOW);   // Activate down movement
  delay(duration);
  digitalWrite(DOWN_PIN, HIGH);  // Deactivate
  
  Serial.println("DOWN movement complete");
}

void emergencyStop() {
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(DOWN_PIN, HIGH);
  Serial.println("EMERGENCY STOP - All movement disabled");
}
