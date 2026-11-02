#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>

// Known-working HW-364 setup: software-I2C clock GPIO14, data GPIO12.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
    U8G2_R0, /* clock=*/14, /* data=*/12, U8X8_PIN_NONE);

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;

uint32_t lastFrameAt = 0;
uint16_t frameNumber = 0;

int16_t wrapX(int16_t x) {
  if (x < -24) return SCREEN_WIDTH + 24;
  if (x > SCREEN_WIDTH + 24) return -24;
  return x;
}

void drawSurface(uint8_t phase) {
  u8g2.setDrawColor(1);
  for (int16_t x = 0; x < SCREEN_WIDTH; x += 2) {
    const int16_t y = 15 + sinf((x + phase * 2) * 0.11f) * 1.5f;
    u8g2.drawPixel(x, y);
    if ((x + phase) % 12 < 2) {
      u8g2.drawPixel(x, y + 1);
    }
  }
  u8g2.drawHLine(0, 18, SCREEN_WIDTH);
}

void drawPlant(int16_t x, uint8_t height, uint8_t sway) {
  const int16_t baseY = 62;
  const int16_t topY = baseY - height;
  const int16_t bend = sinf(sway * 0.09f + x) * 4;
  u8g2.drawLine(x, baseY, x + bend, topY);
  for (uint8_t leaf = 0; leaf < 4; leaf++) {
    const int16_t y = baseY - 5 - leaf * (height / 5);
    const int16_t stemX = x + bend * (baseY - y) / height;
    const int8_t direction = (leaf & 1) ? 1 : -1;
    u8g2.drawLine(stemX, y, stemX + direction * 7, y - 4);
    u8g2.drawLine(stemX + direction * 7, y - 4, stemX + direction * 4, y + 1);
    u8g2.drawLine(stemX + direction * 4, y + 1, stemX, y);
  }
}

void drawBubble(int16_t x, uint8_t phase, uint8_t speed, uint8_t size) {
  const uint8_t travel = (phase * speed + x * 3) % 48;
  const int16_t y = 60 - travel;
  const int16_t drift = sinf((phase + x) * 0.13f) * 2;
  const int16_t bubbleX = x + drift;
  if (y > 19) {
    u8g2.drawCircle(bubbleX, y, size);
    if (size > 2) {
      u8g2.drawPixel(bubbleX - 1, y - 1);
    }
  }
}

void drawGoldfish(int16_t x, int16_t y, bool faceRight, uint8_t size, uint8_t tailPhase) {
  const int8_t direction = faceRight ? 1 : -1;
  const int16_t bodyLength = size * 2;
  const int16_t bodyHeight = size;
  const int16_t tailX = x - direction * bodyLength;
  const int16_t tailWag = (tailPhase & 1) ? 2 : -2;

  u8g2.setDrawColor(1);
  u8g2.drawTriangle(tailX, y,
                    tailX - direction * (size + 5), y - size + tailWag,
                    tailX - direction * (size + 5), y + size + tailWag);
  u8g2.drawFilledEllipse(x, y, bodyLength, bodyHeight);

  // Cut a few scales from the silhouette to suggest bright gold markings.
  u8g2.setDrawColor(0);
  u8g2.drawDisc(x - direction * 3, y - 1, 1);
  u8g2.drawDisc(x + direction * 2, y + 1, 1);
  u8g2.drawDisc(x + direction * 7, y - 1, 1);
  u8g2.setDrawColor(1);

  const int16_t eyeX = x + direction * (bodyLength - 3);
  u8g2.drawDisc(eyeX, y - 2, 1);
  u8g2.drawLine(x - direction * 1, y + bodyHeight - 1,
                x + direction * 5, y + bodyHeight + 2);
}

void drawTank(uint8_t phase) {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(4, 9, "GOLDFISH TANK");
  u8g2.drawStr(101, 9, "<><");
  drawSurface(phase);

  drawBubble(18, phase, 2, 1);
  drawBubble(43, phase + 13, 1, 2);
  drawBubble(88, phase + 25, 2, 1);
  drawBubble(111, phase + 7, 1, 2);

  const int16_t fishX1 = wrapX(20 + (phase * 2) % 150);
  const int16_t fishY1 = 34 + sinf(phase * 0.10f) * 4;
  drawGoldfish(fishX1, fishY1, true, 5, phase / 2);

  const int16_t fishX2 = wrapX(104 - (phase * 1) % 152);
  const int16_t fishY2 = 47 + sinf(phase * 0.13f + 2) * 3;
  drawGoldfish(fishX2, fishY2, false, 3, phase / 3);

  drawPlant(13, 21, phase);
  drawPlant(29, 15, phase + 12);
  drawPlant(101, 17, phase + 28);
  drawPlant(118, 24, phase + 7);

  for (uint8_t pebble = 0; pebble < 18; pebble++) {
    const int16_t x = 3 + pebble * 7;
    const uint8_t radius = 1 + (pebble % 3 == 0);
    u8g2.drawDisc(x, 62 - radius, radius);
  }
  u8g2.drawHLine(0, 63, SCREEN_WIDTH);
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  Serial.println(F("Goldfish tank simulator ready"));
}

void loop() {
  const uint32_t now = millis();
  if (now - lastFrameAt >= 70) {
    lastFrameAt = now;
    drawTank(frameNumber++);
  }
}
