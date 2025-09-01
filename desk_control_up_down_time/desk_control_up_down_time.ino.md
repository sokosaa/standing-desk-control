# Desk Control Setup

## Overview
This Arduino sketch allows you to control a standing desk by sending up/down commands via serial interface. The Arduino simulates button presses on the desk controller.

## Physical Setup Required

### Hardware Components
- Arduino UNO R4 WiFi
- Standing desk with RJ45 controller interface
- RJ45 breakout board or splitter
- RJ45 cables
- Jumper wires

### Wiring Configuration

#### RJ45 Pin Connections
Connect the following RJ45 pins to Arduino pins:

| RJ45 Pin | Function | Arduino Pin | Description |
|----------|----------|-------------|-------------|
| Pin 1 | Down Control | Digital Pin 9 | Pull LOW to activate down movement |
| Pin 2 | Up Control | Digital Pin 8 | Pull LOW to activate up movement |
| Pin 6 | Ground | GND | Common ground connection |

#### Other RJ45 Pins (for reference)
| RJ45 Pin | Function | Status | Notes |
|----------|----------|--------|-------|
| Pin 3 | Preset Buttons | Monitor only | Goes low when preset buttons pressed |
| Pin 4 | 5V Power | Not connected | Always high, likely 5V supply |
| Pin 5 | TX Data | Not connected | Serial transmit (desk to controller) |
| Pin 7 | RX Data | Monitor only | Serial receive (controller to desk) |
| Pin 8 | M Button | Monitor only | Settings/memory button |

### Setup Steps
1. **Disconnect power** from both the desk and Arduino before making connections
2. Connect the standing desk controller pad to one side of the RJ45 splitter
3. Connect the desk control box to the other side of the RJ45 splitter
4. Connect jumper wires from the splitter breakout to Arduino:
   - RJ45 Pin 1 → Arduino Pin 9 (Down control)
   - RJ45 Pin 2 → Arduino Pin 8 (Up control) 
   - RJ45 Pin 6 → Arduino GND
5. Upload the `desk_control.ino` sketch to the Arduino
6. Power on the desk and Arduino

## Usage

### Serial Monitor Commands
Open the Arduino Serial Monitor at **115200 baud** and send these commands:

| Command | Action | Example |
|---------|--------|---------|
| `u` or `up` | Move up for 1 second | `u` |
| `d` or `down` | Move down for 1 second | `d` |
| `u[time]` | Move up for specified milliseconds | `u2000` |
| `d[time]` | Move down for specified milliseconds | `d500` |
| `s` or `stop` | Emergency stop | `s` |

### Safety Notes
- Maximum movement duration is limited to 10 seconds per command
- Always test with short durations first (100-500ms)
- Keep the original controller connected and accessible for manual override
- The `s` command will immediately stop all movement

### Opening Serial Monitor
```bash
arduino-cli monitor -p COM9 -c baudrate=115200
```

## How It Works
The sketch works by simulating the button presses from the original controller:
- When you press the UP button on the controller, RJ45 Pin 2 goes LOW
- When you press the DOWN button on the controller, RJ45 Pin 1 goes LOW
- The Arduino replicates this behavior by pulling the appropriate pins LOW for the specified duration

## Troubleshooting
- **No movement**: Check wiring connections, ensure proper ground connection
- **Continuous movement**: Issue emergency stop command (`s`) and check for stuck signals
- **Serial monitor not working**: Ensure correct COM port and 115200 baud rate
- **Commands not recognized**: Check for proper line endings in serial monitor

## Circuit Diagram
```
Standing Desk Controller Pad
           |
           | (RJ45 Cable)
           |
    RJ45 Splitter/Breakout
           |
    Pin 1 ─────────────────── Arduino Pin 9 (Down)
    Pin 2 ─────────────────── Arduino Pin 8 (Up)
    Pin 6 ─────────────────── Arduino GND
           |
           | (RJ45 Cable)
           |
    Desk Control Box
```
