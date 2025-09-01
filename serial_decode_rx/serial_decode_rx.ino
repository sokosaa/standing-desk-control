#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial.println("Height in inches (RJ7, 9600 baud):");
}

void loop() {
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
        }
      }
      index = 0; // Reset for next packet
    }
  }
}