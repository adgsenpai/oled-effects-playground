#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>
#include <string.h>

// Known-working HW-364 setup: software-I2C clock GPIO14, data GPIO12.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
    U8G2_R0, /* clock=*/14, /* data=*/12, U8X8_PIN_NONE);

constexpr uint8_t WIDTH = 128;
constexpr uint8_t HEIGHT = 64;
constexpr uint8_t TUBE_STEPS = 28;
constexpr uint8_t RING_STEPS = 64;
constexpr float PI_F = 3.14159265f;

uint16_t depthBuffer[WIDTH * HEIGHT];
uint8_t shadeBuffer[WIDTH * HEIGHT];
float tubeCos[TUBE_STEPS];
float tubeSin[TUBE_STEPS];
float ringCos[RING_STEPS];
float ringSin[RING_STEPS];
uint32_t lastFrameAt = 0;
uint16_t frameNumber = 0;

void prepareTables() {
  for (uint8_t i = 0; i < TUBE_STEPS; i++) {
    const float angle = 2.0f * PI_F * i / TUBE_STEPS;
    tubeCos[i] = cosf(angle);
    tubeSin[i] = sinf(angle);
  }
  for (uint8_t i = 0; i < RING_STEPS; i++) {
    const float angle = 2.0f * PI_F * i / RING_STEPS;
    ringCos[i] = cosf(angle);
    ringSin[i] = sinf(angle);
  }
}

void renderDonut(float angleA, float angleB) {
  memset(depthBuffer, 0, sizeof(depthBuffer));
  memset(shadeBuffer, 0, sizeof(shadeBuffer));

  const float sinA = sinf(angleA);
  const float cosA = cosf(angleA);
  const float sinB = sinf(angleB);
  const float cosB = cosf(angleB);

  // Rotate a sampled torus, project it, then keep the nearest surface per pixel.
  for (uint8_t ti = 0; ti < TUBE_STEPS; ti++) {
    const float tubeC = tubeCos[ti];
    const float tubeS = tubeSin[ti];
    const float radial = 1.15f + 0.62f * tubeC;

    for (uint8_t ri = 0; ri < RING_STEPS; ri++) {
      const float ringC = ringCos[ri];
      const float ringS = ringSin[ri];
      const float x = radial * ringC;
      const float y = radial * ringS;
      const float z = 0.62f * tubeS;

      const float rotatedX = x * cosB - y * sinB;
      const float rotatedY = (x * sinB + y * cosB) * cosA - z * sinA;
      const float rotatedZ = (x * sinB + y * cosB) * sinA + z * cosA;
      const float distance = 4.2f - rotatedZ;
      if (distance <= 0.1f) {
        continue;
      }

      const int16_t px = 64 + (int16_t)(100.0f * rotatedX / distance);
      const int16_t py = 36 - (int16_t)(52.0f * rotatedY / distance);
      if (px < 1 || px >= WIDTH - 1 || py < 15 || py >= HEIGHT - 1) {
        continue;
      }

      const uint16_t depth = 6000 - (uint16_t)(distance * 1000.0f);
      const uint16_t index = py * WIDTH + px;
      if (depth <= depthBuffer[index]) {
        continue;
      }
      depthBuffer[index] = depth;

      // Rotate the torus normal and quantize its light level for monochrome dithering.
      const float nx = tubeC * ringC;
      const float ny = tubeC * ringS;
      const float nz = tubeS;
      const float normalX = nx * cosB - ny * sinB;
      const float normalY = (nx * sinB + ny * cosB) * cosA - nz * sinA;
      const float normalZ = (nx * sinB + ny * cosB) * sinA + nz * cosA;
      float light = normalX * -0.35f + normalY * -0.45f + normalZ * 0.82f;
      if (light < 0.0f) {
        light = 0.0f;
      }
      if (light > 1.0f) {
        light = 1.0f;
      }
      shadeBuffer[index] = 1 + (uint8_t)(light * 3.0f);
    }
  }

  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(4, 9, "ESP8266 / SPINNING DONUT");
  u8g2.drawHLine(3, 12, 122);

  static const uint8_t dither[2][2] = {{0, 2}, {3, 1}};
  for (uint8_t y = 15; y < HEIGHT - 1; y++) {
    for (uint8_t x = 1; x < WIDTH - 1; x++) {
      const uint16_t index = y * WIDTH + x;
      if (shadeBuffer[index] > dither[y & 1][x & 1]) {
        u8g2.drawPixel(x, y);
      }
    }
  }
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  prepareTables();
  Serial.println(F("Spinning monochrome 3D donut"));
}

void loop() {
  const uint32_t now = millis();
  if (now - lastFrameAt >= 45) {
    lastFrameAt = now;
    const float phase = frameNumber * 0.055f;
    renderDonut(phase * 0.73f, phase);
    frameNumber++;
  }
}
