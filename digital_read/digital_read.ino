// Arduino Monitor for RJ45 Pins (Digital, Labeled, Reverse Pin Mapping)
const int pinMap[8] = {9, 8, 7, 6, 5, 4, 3, 2}; // RJ45 Pin 1->Arduino Pin 9, ..., RJ45 Pin 8->Arduino Pin 2
const char* pinLabels[8] = {"RJ1", "RJ2", "RJ3", "RJ4", "RJ5", "RJ6", "RJ7", "RJ8"};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(pinMap[i], INPUT); // Set as inputs
  }
  Serial.println("Monitoring RJ45 pins (0=Low, 1=High):");
  for (int i = 0; i < 8; i++) {
    Serial.print(pinLabels[i]);
    Serial.print("\t");
  }
  Serial.println();
}

void loop() {
  // Print digital states every 500ms
  static unsigned long lastSample = 0;
  if (millis() - lastSample >= 500) {
    for (int i = 0; i < 8; i++) {
      Serial.print(pinLabels[i]);
      Serial.print(": ");
      Serial.print(digitalRead(pinMap[i]));
      Serial.print("\t");
    }
    Serial.println();
    lastSample = millis();
  }
}