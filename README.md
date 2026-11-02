# Spinning Donut OLED

A continuously spinning, shaded 3D torus animation for the monochrome 128x64 SSD1306 OLED built into an ESP8266 HW-364 / D-duino-style board.

The display cannot produce RGB colors. This sketch uses an ordered-dither pattern to create several apparent brightness levels from its black-and-white pixels.

## Board and libraries

- ESP8266 NodeMCU-based HW-364 board
- Built-in SSD1306 128x64 OLED
- U8g2 library by olikraus
- OLED software-I²C clock: GPIO14; data: GPIO12

Install the ESP8266 Arduino core and U8g2 library, select a compatible ESP8266 board profile, and upload `oled-effects-playground.ino`. Use serial monitor at 115200 baud.

Arduino CLI example:

```sh
arduino-cli core install esp8266:esp8266
arduino-cli lib install U8g2
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 --port /dev/ttyUSB0 .
```
