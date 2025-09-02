# Smart Desk Google Home Setup

## Quick Setup Instructions:

1. **Update WiFi credentials** in the sketch:
   - Change `YOUR_WIFI_SSID` to your actual WiFi network name
   - Change `YOUR_WIFI_PASSWORD` to your actual WiFi password

2. **Upload the sketch** to your Arduino R4 Uno WiFi

3. **Get the IP address** from the Serial Monitor

4. **Test with browser** first:
   - http://YOUR_ARDUINO_IP/on - Turn on (happy face)
   - http://YOUR_ARDUINO_IP/off - Turn off
   - http://YOUR_ARDUINO_IP/happy - Show happy face  
   - http://YOUR_ARDUINO_IP/heart - Show heart

5. **Google Home Integration** (No hub required):
   - Use IFTTT (If This Then That) app
   - Create applets like:
     - "Hey Google, desk happy" → Webhooks → http://YOUR_ARDUINO_IP/happy
     - "Hey Google, desk heart" → Webhooks → http://YOUR_ARDUINO_IP/heart
     - "Hey Google, turn on desk" → Webhooks → http://YOUR_ARDUINO_IP/on
     - "Hey Google, turn off desk" → Webhooks → http://YOUR_ARDUINO_IP/off

## Features:
- ✅ WiFi enabled LED matrix control
- ✅ Web interface for testing
- ✅ RESTful API for Google Home
- ✅ Happy face and heart patterns
- ✅ Simple on/off states
- ✅ Status monitoring

## Commands:
- `/on` - Turn on with happy face
- `/off` - Turn off LEDs  
- `/happy` - Show happy face
- `/heart` - Show heart
- `/status` - Get current state (JSON)
