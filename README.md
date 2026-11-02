# OLED Effects Playground

Eight animated effects for the monochrome 128x64 SSD1306 OLED built into the ESP8266 HW-364 / D-duino-style board. A new effect starts every five seconds.

The display is monochrome, so it cannot produce RGB colors. This project explores the available visual range with dithering, black-and-white patterns, geometric animation, and motion.

## Effects

1. Dither spectrum
2. Starfield
3. Radar sweep
4. Wave interference
5. Kaleidoscope
6. Digital rain
7. Particle burst
8. Living checkerboard

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
