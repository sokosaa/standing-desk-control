// Pin mappings: RJ45 Pin 1->Arduino Pin 9, ..., RJ45 Pin 8->Arduino Pin 2, RJ45 Pin 7->Arduino Pin 0
const int pinMap[8] = {9, 8, 7, 6, 5, 4, 0, 2};
const char* pinLabels[8] = {"RJ1", "RJ2", "RJ3", "RJ4", "RJ5", "RJ6", "RJ7", "RJ8"};

void setup() {
  Serial.begin(115200); // Serial Monitor
  Serial1.begin(9600); // RJ7 (RX, Pin 7 on Arduino Pin 0)
  for (int i = 0; i < 8; i++) {
    if (pinMap[i] != 0) { // Skip RJ7 (on pin 0)
      pinMode(pinMap[i], INPUT);
    }
  }
  Serial.println("Monitoring RJ45 pins (0=Low, 1=High, RJ7 via Serial1 at 9600 baud):");
  for (int i = 0; i < 8; i++) {
    Serial.print(pinLabels[i]);
    Serial.print("\t");
  }
  Serial.println();
}

void loop() {
  // Check serial data on RJ7
  if (Serial1.available()) {
    Serial.print("Serial RJ7 (9600): ");
    while (Serial1.available()) {
      char c = Serial1.read();
      if (isPrintable(c)) {
        Serial.print(c);
      } else {
        Serial.print("[0x");
        if (c < 16) Serial.print("0");
        Serial.print(c, HEX);
        Serial.print("]");
      }
    }
    Serial.println();
  }

  // Digital state monitoring (every 500ms, skip RJ7)
  static unsigned long lastSample = 0;
  if (millis() - lastSample >= 500) {
    for (int i = 0; i < 8; i++) {
      Serial.print(pinLabels[i]);
      Serial.print(": ");
      if (pinMap[i] == 0) {
        Serial.print("N/A"); // RJ7 on Serial1
      } else {
        Serial.print(digitalRead(pinMap[i]));
      }
      Serial.print("\t");
    }
    Serial.println();
    lastSample = millis();
  }
}