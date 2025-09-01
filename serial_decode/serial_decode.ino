// Pin mappings: RJ45 Pin 1->Pin 9, ..., Pin 8->Pin 2, Pin 4->Pin 0
const int pinMap[8] = {9, 8, 7, 0, 5, 4, 3, 2};
const char* pinLabels[8] = {"RJ1", "RJ2", "RJ3", "RJ4", "RJ5", "RJ6", "RJ7", "RJ8"};

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600); // RJ4 (possible TX, Pin 4 on Arduino Pin 0)
  for (int i = 0; i < 8; i++) {
    if (pinMap[i] != 0) {
      pinMode(pinMap[i], INPUT);
    }
  }
  Serial.println("Monitoring RJ45 pins (0=Low, 1=High, RJ4 via Serial1 at 9600 baud):");
  for (int i = 0; i < 8; i++) {
    Serial.print(pinLabels[i]);
    Serial.print("\t");
  }
  Serial.println();
}

void loop() {
  // Check serial data on RJ4
  if (Serial1.available()) {
    Serial.print("Serial RJ4 (9600): ");
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

  // Digital state monitoring (every 500ms)
  static unsigned long lastSample = 0;
  if (millis() - lastSample >= 500) {
    for (int i = 0; i < 8; i++) {
      Serial.print(pinLabels[i]);
      Serial.print(": ");
      if (pinMap[i] == 0) {
        Serial.print("N/A");
      } else {
        Serial.print(digitalRead(pinMap[i]));
      }
      Serial.print("\t");
    }
    Serial.println();
    lastSample = millis();
  }
}