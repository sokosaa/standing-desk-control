// Smart Standing Desk Controller
// Reads current height via RX (Pin 7) and sends movement commands via TX (Pin 5)
// Can move desk to specific target heights

#include <Arduino.h>

// Pin definitions based on your setup
const int DOWN_PIN = 9;      // RJ45 Pin 1 - goes low to move down
const int UP_PIN = 8;        // RJ45 Pin 2 - goes low to move up
const int TX_PIN = 5;        // RJ45 Pin 5 - TX data line
const int RX_PIN = 0;        // RJ45 Pin 7 - RX data line (connected to Serial1)

// Height tracking variables
float currentHeight = 0.0;
float targetHeight = 0.0;
bool movingToTarget = false;
unsigned long moveStartTime = 0;
const unsigned long MAX_MOVE_TIME = 30000; // 30 second safety timeout
const float HEIGHT_TOLERANCE = 0.05; // Stop when within 0.05 inches of target
const float MIN_HEIGHT = 25.0; // Minimum safe height
const float MAX_HEIGHT = 50.5; // Maximum safe height

// Movement control
bool isMovingUp = false;
bool isMovingDown = false;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600); // For reading height data from desk
  
  // Set up control pins
  pinMode(DOWN_PIN, OUTPUT);
  pinMode(UP_PIN, OUTPUT);
  digitalWrite(DOWN_PIN, HIGH); // Inactive
  digitalWrite(UP_PIN, HIGH);   // Inactive
  
  Serial.println("Smart Standing Desk Controller Ready");
  Serial.println("Commands:");
  Serial.println("  'h' or 'height' - Show current height");
  Serial.println("  'goto [height]' - Move to specific height (e.g., 'goto 42.5')");
  Serial.println("  'up [height]' - Move up to height (e.g., 'up 45')");
  Serial.println("  'down [height]' - Move down to height (e.g., 'down 30')");
  Serial.println("  'stop' - Stop all movement");
  Serial.println("  'u' - Manual up for 1 second");
  Serial.println("  'd' - Manual down for 1 second");
  Serial.println("  'reset' - Perform desk hard reset (ASR mode)");
  Serial.print("  Safe height range: ");
  Serial.print(MIN_HEIGHT);
  Serial.print(" to ");
  Serial.print(MAX_HEIGHT);
  Serial.println(" inches");
  Serial.println();
}

void loop() {
  // Read height data from desk
  readHeightData();
  
  // Handle automatic movement to target - check this VERY frequently
  handleAutomaticMovement();
  
  // Check for serial commands
  handleSerialCommands();
  
  // Safety timeout check
  if (movingToTarget && (millis() - moveStartTime > MAX_MOVE_TIME)) {
    stopMovement();
    Serial.println("ERROR: Movement timeout - stopping for safety");
  }
  
  // Very short delay to keep loop responsive
  delay(10);
}

void readHeightData() {
  static uint8_t buffer[4];
  static int index = 0;
  static uint16_t lastHeight = 0xFFFF;

  if (Serial1.available()) {
    buffer[index] = Serial1.read();
    index++;
    if (index == 4) { // Full packet
      if (buffer[0] == 0x01 && buffer[1] == 0x01) { // Valid height packet
        uint16_t height = (buffer[2] << 8) | buffer[3];
        float newHeight = height / 10.0; // Tenths of an inch
        
        // Filter out obviously invalid readings (over 100 inches)
        if (newHeight < 100.0 && newHeight > 20.0) {
          if (height != lastHeight) {
            currentHeight = newHeight;
            lastHeight = height;
            
            // IMMEDIATE safety check on every height update
            if (movingToTarget) {
              // Check if we're getting close to target - stop early
              float heightDiff = targetHeight - currentHeight;
              if ((isMovingUp && heightDiff <= 0.1) || 
                  (isMovingDown && heightDiff >= -0.1)) {
                stopMovement();
                Serial.print("EARLY STOP! Height: ");
                Serial.print(currentHeight);
                Serial.print(" Target: ");
                Serial.println(targetHeight);
                return;
              }
            }
            
            if (!movingToTarget || (millis() % 500 == 0)) { // Reduce spam during movement
              Serial.print("Current Height: ");
              Serial.print(currentHeight);
              Serial.println(" inches");
            }
          }
        }
      }
      index = 0; // Reset for next packet
    }
  }
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "h" || command == "height") {
      Serial.print("Current Height: ");
      Serial.print(currentHeight);
      Serial.println(" inches");
    }
    else if (command.startsWith("goto ")) {
      float target = command.substring(5).toFloat();
      moveToHeight(target);
    }
    else if (command.startsWith("up ")) {
      float target = command.substring(3).toFloat();
      if (target > currentHeight) {
        moveToHeight(target);
      } else {
        Serial.println("Target height must be higher than current height");
      }
    }
    else if (command.startsWith("down ")) {
      float target = command.substring(5).toFloat();
      if (target < currentHeight) {
        moveToHeight(target);
      } else {
        Serial.println("Target height must be lower than current height");
      }
    }
    else if (command == "stop") {
      stopMovement();
      Serial.println("Movement stopped");
    }
    else if (command == "u") {
      manualMove(true, 1000);
    }
    else if (command == "d") {
      manualMove(false, 1000);
    }
    else if (command == "reset") {
      performHardReset();
    }
    else {
      Serial.println("Unknown command. Use 'h', 'goto [height]', 'up [height]', 'down [height]', 'stop', 'u', 'd', or 'reset'");
    }
  }
}

void moveToHeight(float target) {
  if (target < MIN_HEIGHT || target > MAX_HEIGHT) {
    Serial.print("ERROR: Target height must be between ");
    Serial.print(MIN_HEIGHT);
    Serial.print(" and ");
    Serial.print(MAX_HEIGHT);
    Serial.println(" inches");
    return;
  }
  
  // Extra safety check against current height
  if (currentHeight > 0 && currentHeight < MIN_HEIGHT) {
    Serial.println("ERROR: Current height below minimum - perform reset first");
    return;
  }
  
  if (currentHeight > MAX_HEIGHT) {
    Serial.println("ERROR: Current height above maximum - perform reset first");
    return;
  }
  
  targetHeight = target;
  movingToTarget = true;
  moveStartTime = millis();
  
  Serial.print("Moving to target height: ");
  Serial.print(targetHeight);
  Serial.println(" inches");
}

void handleAutomaticMovement() {
  if (!movingToTarget) return;
  
  float heightDiff = targetHeight - currentHeight;
  
  // CRITICAL: Immediate safety checks
  if (currentHeight < MIN_HEIGHT - 0.5 || currentHeight > MAX_HEIGHT + 0.5) {
    stopMovement();
    Serial.print("SAFETY STOP! Height out of bounds: ");
    Serial.print(currentHeight);
    Serial.println(" inches");
    return;
  }
  
  // CRITICAL: Very aggressive overshoot detection
  if (isMovingUp && currentHeight >= targetHeight) {
    stopMovement();
    Serial.print("UP OVERSHOOT STOP! Height: ");
    Serial.print(currentHeight);
    Serial.print(" Target: ");
    Serial.println(targetHeight);
    return;
  }
  
  if (isMovingDown && currentHeight <= targetHeight) {
    stopMovement();
    Serial.print("DOWN OVERSHOOT STOP! Height: ");
    Serial.print(currentHeight);
    Serial.print(" Target: ");
    Serial.println(targetHeight);
    return;
  }
  
  // Check if we've reached the target with tolerance
  if (fabs(heightDiff) <= HEIGHT_TOLERANCE) {
    stopMovement();
    Serial.print("TARGET REACHED! Final height: ");
    Serial.print(currentHeight);
    Serial.println(" inches");
    return;
  }
  
  // Determine direction and move
  if (heightDiff > 0 && !isMovingUp) {
    // Need to go up
    stopMovement();
    startMovingUp();
    Serial.print("Moving UP... Current: ");
    Serial.print(currentHeight);
    Serial.print(" Target: ");
    Serial.println(targetHeight);
  }
  else if (heightDiff < 0 && !isMovingDown) {
    // Need to go down
    stopMovement();
    startMovingDown();
    Serial.print("Moving DOWN... Current: ");
    Serial.print(currentHeight);
    Serial.print(" Target: ");
    Serial.println(targetHeight);
  }
}

void startMovingUp() {
  digitalWrite(UP_PIN, LOW);
  isMovingUp = true;
  isMovingDown = false;
}

void startMovingDown() {
  digitalWrite(DOWN_PIN, LOW);
  isMovingUp = false;
  isMovingDown = true;
}

void stopMovement() {
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(DOWN_PIN, HIGH);
  isMovingUp = false;
  isMovingDown = false;
  movingToTarget = false;
}

void manualMove(bool up, int duration) {
  if (movingToTarget) {
    Serial.println("Cannot manual move while auto-moving to target. Use 'stop' first.");
    return;
  }
  
  Serial.print("Manual move ");
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
  
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(DOWN_PIN, HIGH);
  
  Serial.println("Manual move complete");
}

void performHardReset() {
  if (movingToTarget) {
    Serial.println("Cannot reset while auto-moving. Use 'stop' first.");
    return;
  }
  
  Serial.println("PERFORMING HARD RESET - This will take about 30 seconds");
  Serial.println("Step 1: Moving to lowest position...");
  
  // Move down until ASR appears (about 25 seconds max)
  digitalWrite(DOWN_PIN, LOW);
  unsigned long resetStart = millis();
  
  while (millis() - resetStart < 25000) {
    readHeightData(); // Keep reading height
    if (currentHeight < MIN_HEIGHT - 1.0) {
      Serial.println("Step 2: ASR mode detected - release and hold again...");
      break;
    }
    delay(100);
  }
  
  // Release briefly
  digitalWrite(DOWN_PIN, HIGH);
  delay(500);
  
  Serial.println("Step 3: Continuing reset - going below minimum...");
  // Hold down again for the reset sequence
  digitalWrite(DOWN_PIN, LOW);
  delay(8000); // Hold for 8 more seconds
  
  // Release - desk should bump back up to 25"
  digitalWrite(DOWN_PIN, HIGH);
  Serial.println("Step 4: Reset complete - desk should return to 25 inches");
  Serial.println("Wait for desk to stabilize...");
}
