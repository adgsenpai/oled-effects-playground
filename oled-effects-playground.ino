#include <Arduino.h>
#include <U8g2lib.h>

// Known-working HW-364 setup: software-I2C clock GPIO14, data GPIO12.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
    U8G2_R0, /* clock=*/14, /* data=*/12, U8X8_PIN_NONE);

constexpr uint32_t EFFECT_DURATION_MS = 5000;
constexpr uint32_t FRAME_INTERVAL_MS = 40;
constexpr uint8_t EFFECT_COUNT = 8;

uint32_t effectStartedAt = 0;
uint32_t lastFrameAt = 0;
uint8_t currentEffect = 0;
uint8_t frameNumber = 0;
uint32_t randomState = 0xC0FFEEu;

uint32_t nextRandom() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;
  return randomState;
}

void drawHeader(const char *name) {
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(3, 8, name);
  u8g2.drawHLine(2, 11, 124);
  u8g2.drawFrame(2, 13, 124, 49);
}

void effectSpectrum() {
  drawHeader("01 / DITHER SPECTRUM");
  for (uint8_t band = 0; band < 6; band++) {
    const uint8_t y0 = 17 + band * 7;
    for (uint8_t x = 5; x < 123; x++) {
      const uint8_t threshold = (x * 3 + band * 29 + frameNumber * 2) & 63;
      if (threshold < (band + 1) * 9) {
        u8g2.drawPixel(x, y0 + ((x + frameNumber) % 5));
      }
    }
  }
}

void effectStarfield() {
  drawHeader("02 / STARFIELD");
  for (uint8_t i = 0; i < 38; i++) {
    const uint32_t seed = i * 2654435761UL;
    const uint8_t x = 5 + ((seed + frameNumber * (1 + i % 3)) % 118);
    const uint8_t y = 16 + ((seed >> 8) % 44);
    const uint8_t size = (i % 9 == 0) ? 2 : 1;
    u8g2.drawDisc(x, y, size);
  }
  u8g2.drawDisc(64, 39, 3);
}

void effectRadar() {
  drawHeader("03 / RADAR SWEEP");
  constexpr int16_t cx = 64;
  constexpr int16_t cy = 39;
  u8g2.drawCircle(cx, cy, 9);
  u8g2.drawCircle(cx, cy, 18);
  u8g2.drawCircle(cx, cy, 27);
  const float angle = (frameNumber % 64) * 0.09817477f;
  const int16_t dx = cosf(angle) * 27;
  const int16_t dy = sinf(angle) * 27;
  u8g2.drawLine(cx, cy, cx + dx, cy + dy);
  u8g2.drawDisc(cx + cosf(angle) * 18, cy + sinf(angle) * 18, 2);
}

void effectWave() {
  drawHeader("04 / WAVE INTERFERENCE");
  for (int16_t x = 5; x < 123; x += 2) {
    const float phase = x * 0.12f + frameNumber * 0.18f;
    const int16_t y1 = 38 + sinf(phase) * 11;
    const int16_t y2 = 38 + cosf(phase * 0.8f) * 9;
    u8g2.drawPixel(x, y1);
    u8g2.drawPixel(x, y2);
    if ((x & 7) == 5) {
      u8g2.drawLine(x, y1, x, y2);
    }
  }
}

void effectKaleidoscope() {
  drawHeader("05 / KALEIDOSCOPE");
  const int16_t cx = 64;
  const int16_t cy = 39;
  for (uint8_t i = 0; i < 8; i++) {
    const float a = (i * 0.785398f) + frameNumber * 0.08f;
    const int16_t x = cx + cosf(a) * (12 + (i % 3) * 5);
    const int16_t y = cy + sinf(a) * (12 + (i % 3) * 5);
    u8g2.drawLine(cx, cy, x, y);
    u8g2.drawCircle(x, y, 3 + (i % 3));
  }
  u8g2.drawDisc(cx, cy, 4);
}

void effectMatrix() {
  drawHeader("06 / DIGITAL RAIN");
  for (uint8_t column = 0; column < 16; column++) {
    const uint8_t x = 6 + column * 7;
    const uint8_t head = (frameNumber * (1 + column % 3) + column * 11) % 47;
    for (uint8_t trail = 0; trail < 5; trail++) {
      const uint8_t y = 16 + ((head + trail * 7) % 45);
      if (nextRandom() % (trail + 2) != 0) {
        u8g2.drawBox(x, y, 3, 3);
      }
    }
  }
}

void effectParticles() {
  drawHeader("07 / PARTICLE BURST");
  const uint8_t pulse = 8 + ((frameNumber * 3) % 20);
  u8g2.drawCircle(64, 39, pulse);
  u8g2.drawCircle(64, 39, pulse / 2);
  for (uint8_t i = 0; i < 32; i++) {
    const float angle = i * 0.1963495f + frameNumber * 0.045f;
    const uint8_t radius = (i * 13 + frameNumber * (1 + i % 4)) % 28;
    const int16_t x = 64 + cosf(angle) * radius;
    const int16_t y = 39 + sinf(angle) * radius;
    if (i % 4 == 0) {
      u8g2.drawDisc(x, y, 1);
    } else {
      u8g2.drawPixel(x, y);
    }
  }
}

void effectChecker() {
  drawHeader("08 / LIVING CHECKER");
  for (uint8_t row = 0; row < 6; row++) {
    for (uint8_t col = 0; col < 15; col++) {
      if (((row + col + frameNumber / 3) & 1) == 0) {
        u8g2.drawBox(6 + col * 8, 17 + row * 7, 7, 6);
      }
    }
  }
  u8g2.setDrawColor(2);
  u8g2.drawDisc(64 + sinf(frameNumber * 0.12f) * 45,
                40 + cosf(frameNumber * 0.16f) * 19, 5);
  u8g2.setDrawColor(1);
}

void drawCurrentEffect() {
  u8g2.clearBuffer();
  switch (currentEffect) {
    case 0: effectSpectrum(); break;
    case 1: effectStarfield(); break;
    case 2: effectRadar(); break;
    case 3: effectWave(); break;
    case 4: effectKaleidoscope(); break;
    case 5: effectMatrix(); break;
    case 6: effectParticles(); break;
    default: effectChecker(); break;
  }

  char counter[12];
  snprintf(counter, sizeof(counter), "%u/%u", currentEffect + 1, EFFECT_COUNT);
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.setDrawColor(1);
  u8g2.drawStr(105, 9, counter);
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  effectStartedAt = millis();
  Serial.println(F("OLED effects playground: monochrome SSD1306"));
}

void loop() {
  const uint32_t now = millis();
  if (now - effectStartedAt >= EFFECT_DURATION_MS) {
    effectStartedAt += EFFECT_DURATION_MS;
    currentEffect = (currentEffect + 1) % EFFECT_COUNT;
    Serial.printf("Effect %u/%u\n", currentEffect + 1, EFFECT_COUNT);
  }

  if (now - lastFrameAt >= FRAME_INTERVAL_MS) {
    lastFrameAt = now;
    frameNumber++;
    drawCurrentEffect();
  }
}
