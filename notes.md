# Setup
- standing desk controller pad plugged into a rj45 splitter breakout board
- another rj45 cable connecting desk control box to the breakout board 
- arduino r4 uno wifi board hooked up to the rj45 breakout board via the following pin setup

# RJ45 Pins

## 1
- goes low/0 on down hold
- currently connected to arduino digital (pwm~) ~9

## 2
- goes low/0 on up hold
- currently connected to arduino digital (pwm~) 8

## 3
- goes low/0 when holding one of the preset (1-4) buttons on controller
- currently connected to arduino digital (pwm~) 7

## 4
- always high/1
- is likely 5V
- currently connected to nothing

## 5
- always high/1 if digitally read
- is likely Tx
- currently connected to nothing

## 6
- always low/0
- is GND
- currently connected to arduino GND

## 7
- goes low/0 on up or down hold if digitally read
- also goes low for certain button press actions
- is serial / Rx, can be read with `serial_decode_rx.ino`
- currently connected to arduino digital (pwm~) 0 -> RX

## 8
- always high/1 if digitally read
- is likely M button (change settings)
- currently connected to arduino digital (pwm~) 2


# Resources
https://embedded-elixir.com/post/2019-01-18-nerves-at-home-desk-controller/