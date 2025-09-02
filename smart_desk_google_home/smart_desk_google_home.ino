#include <WiFiS3.h>
#include <WiFiServer.h>
#include "Arduino_LED_Matrix.h"

// WiFi credentials - UPDATE THESE
const char* ssid = "El Gato!";
const char* password = "print(f\\\"Today's_car_is_{macka}\\\")";

ArduinoLEDMatrix matrix;
WiFiServer server(80);

// LED Matrix patterns
const uint32_t happy[] = {
  0x19819,
  0x80000001,
  0x81f8000
};

const uint32_t heart[] = {
  0x3184a444,
  0x44042081,
  0x100a0040
};

const uint32_t off[] = {
  0x0,
  0x0,
  0x0
};

// Current state
String currentPattern = "off";
bool deviceState = false;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  
  // Show startup pattern
  matrix.loadFrame(off);
  
  // Connect to WiFi with comprehensive debugging
  Serial.println("=== WiFi Connection Debug ===");
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  Serial.print("Password length: ");
  Serial.println(strlen(password));
  
  // Get MAC address before connecting
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("Arduino MAC Address: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  WiFi.begin(ssid, password);
  Serial.print("WiFi.begin() called, waiting for connection");
  
  int attempts = 0;
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    attempts++;
    Serial.print(".");
    
    // Detailed status every 5 seconds
    if (attempts % 5 == 0) {
      Serial.println();
      Serial.print("Attempt ");
      Serial.print(attempts);
      Serial.print("/30 - Status: ");
      
      switch(WiFi.status()) {
        case WL_IDLE_STATUS:
          Serial.println("WL_IDLE_STATUS (0) - WiFi idle");
          break;
        case WL_NO_SSID_AVAIL:
          Serial.println("WL_NO_SSID_AVAIL (1) - Network not found");
          break;
        case WL_SCAN_COMPLETED:
          Serial.println("WL_SCAN_COMPLETED (2) - Scan completed");
          break;
        case WL_CONNECTED:
          Serial.println("WL_CONNECTED (3) - Connected!");
          break;
        case WL_CONNECT_FAILED:
          Serial.println("WL_CONNECT_FAILED (4) - Connection failed");
          break;
        case WL_CONNECTION_LOST:
          Serial.println("WL_CONNECTION_LOST (5) - Connection lost");
          break;
        case WL_DISCONNECTED:
          Serial.println("WL_DISCONNECTED (6) - Disconnected");
          break;
        default:
          Serial.print("Unknown status: ");
          Serial.println(WiFi.status());
      }
      
      // Try to get partial network info even if not fully connected
      IPAddress ip = WiFi.localIP();
      if (ip != IPAddress(0,0,0,0)) {
        Serial.print("Partial IP obtained: ");
        Serial.println(ip);
      }
    }
  }
  
  unsigned long connectionTime = millis() - startTime;
  Serial.println();
  Serial.print("Connection attempt took: ");
  Serial.print(connectionTime);
  Serial.println(" ms");
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("=== WiFi Connected Successfully ===");
  } else {
    Serial.println("=== WiFi Connection Failed ===");
    Serial.print("Final status: ");
    Serial.println(WiFi.status());
  }
  
  Serial.println();
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status());
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  Serial.print("Signal Strength (RSSI): ");
  Serial.println(WiFi.RSSI());
  
  // DHCP Analysis
  Serial.println("=== DHCP Analysis ===");
  IPAddress localIP = WiFi.localIP();
  IPAddress gateway = WiFi.gatewayIP();
  IPAddress subnet = WiFi.subnetMask();
  IPAddress dns = WiFi.dnsIP();
  
  if (localIP == IPAddress(0,0,0,0)) {
    Serial.println("❌ DHCP FAILED: No IP address assigned");
    Serial.println("Possible causes:");
    Serial.println("  - DHCP server not responding");
    Serial.println("  - DHCP pool exhausted");
    Serial.println("  - MAC address filtering");
    Serial.println("  - Device restrictions");
    
    // Try to reconnect with different approach
    Serial.println("Attempting DHCP renewal...");
    WiFi.disconnect();
    delay(2000);
    WiFi.begin(ssid, password);
    delay(5000);
    
    if (WiFi.localIP() != IPAddress(0,0,0,0)) {
      Serial.print("✅ DHCP renewal successful: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("❌ DHCP renewal failed");
    }
  } else {
    Serial.println("✅ DHCP SUCCESS: Valid IP obtained");
    
    if (gateway == IPAddress(0,0,0,0)) {
      Serial.println("⚠️  WARNING: No gateway - limited connectivity");
    }
    if (dns == IPAddress(0,0,0,0)) {
      Serial.println("⚠️  WARNING: No DNS - domain resolution disabled");
    }
    if (subnet == IPAddress(0,0,0,0)) {
      Serial.println("⚠️  WARNING: No subnet mask");
    }
  }
  
  // Start web server
  server.begin();
  Serial.println("Web server started");
  Serial.println("Commands:");
  Serial.println("  http://" + WiFi.localIP().toString() + "/on - Turn on (happy)");
  Serial.println("  http://" + WiFi.localIP().toString() + "/off - Turn off");
  Serial.println("  http://" + WiFi.localIP().toString() + "/happy - Show happy face");
  Serial.println("  http://" + WiFi.localIP().toString() + "/heart - Show heart");
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected");
    String request = "";
    
    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        request += line;
        
        if (line.length() == 1) { // End of HTTP request
          // Parse the request
          if (request.indexOf("GET /on") >= 0) {
            handleOn();
            sendResponse(client, "OK - Device ON, Happy face displayed");
          }
          else if (request.indexOf("GET /off") >= 0) {
            handleOff();
            sendResponse(client, "OK - Device OFF, LEDs off");
          }
          else if (request.indexOf("GET /happy") >= 0) {
            handleHappy();
            sendResponse(client, "OK - Happy face displayed");
          }
          else if (request.indexOf("GET /heart") >= 0) {
            handleHeart();
            sendResponse(client, "OK - Heart displayed");
          }
          else if (request.indexOf("GET /status") >= 0) {
            handleStatus(client);
          }
          else {
            handleRoot(client);
          }
          break;
        }
      }
    }
    
    client.stop();
    Serial.println("Client disconnected");
  }
}

void sendResponse(WiFiClient client, String message) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.println(message);
}

void handleRoot(WiFiClient client) {
  String html = "<!DOCTYPE html><html><head><title>Smart Desk Control</title></head><body>";
  html += "<h1>Smart Desk LED Control</h1>";
  html += "<p>Current pattern: " + currentPattern + "</p>";
  html += "<p>Device state: " + String(deviceState ? "ON" : "OFF") + "</p>";
  html += "<br><a href='/on'>Turn ON (Happy)</a><br>";
  html += "<a href='/off'>Turn OFF</a><br>";
  html += "<a href='/happy'>Happy Face</a><br>";
  html += "<a href='/heart'>Heart</a><br>";
  html += "</body></html>";
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.println(html);
}

void handleOn() {
  deviceState = true;
  currentPattern = "happy";
  matrix.loadFrame(happy);
  Serial.println("Device turned ON - Happy face displayed");
}

void handleOff() {
  deviceState = false;
  currentPattern = "off";
  matrix.loadFrame(off);
  Serial.println("Device turned OFF - LEDs off");
}

void handleHappy() {
  deviceState = true;
  currentPattern = "happy";
  matrix.loadFrame(happy);
  Serial.println("Happy face displayed");
}

void handleHeart() {
  deviceState = true;
  currentPattern = "heart";
  matrix.loadFrame(heart);
  Serial.println("Heart displayed");
}

void handleStatus(WiFiClient client) {
  String status = "{";
  status += "\"state\":\"" + String(deviceState ? "on" : "off") + "\",";
  status += "\"pattern\":\"" + currentPattern + "\",";
  status += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  status += "}";
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  client.println(status);
}
