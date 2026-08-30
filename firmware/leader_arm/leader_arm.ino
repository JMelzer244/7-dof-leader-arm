#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <math.h>

// ======================================================
// OLED: separate software-I2C bus
// ======================================================
#define OLED_SCL 13
#define OLED_SDA 14
#define OLED_RST 12

U8G2_SSD1309_128X64_NONAME0_F_SW_I2C oled(
  U8G2_R0,
  OLED_SCL,
  OLED_SDA,
  OLED_RST
);

// ======================================================
// TCA9548A: hardware-I2C bus
// ======================================================
#define MUX_SDA 9
#define MUX_SCL 8

#define TCA9548A_ADDRESS 0x70
#define AS5600_ADDRESS 0x36
#define AS5600_RAW_ANGLE_REGISTER 0x0C

#define SENSOR_COUNT 7
#define CALIBRATION_SAMPLES 20

// J1=CH0, J2=CH1, J3=CH7, J4=CH6
// J5=CH5, J6=CH4, J7=CH3
const uint8_t sensorChannels[SENSOR_COUNT] = {
  0, 1, 7, 6, 5, 4, 3
};

// Filled automatically during startup calibration
float zeroOffsetDegrees[SENSOR_COUNT] = {
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

// ======================================================
// Draw centered text
// ======================================================
void drawCenteredText(int y, const char *text) {
  int textWidth = oled.getStrWidth(text);
  int x = (128 - textWidth) / 2;
  oled.drawStr(x, y, text);
}

// ======================================================
// Select one multiplexer channel
// ======================================================
bool selectMuxChannel(uint8_t channel) {
  if (channel > 7) {
    return false;
  }

  Wire.beginTransmission(TCA9548A_ADDRESS);
  Wire.write(1 << channel);

  return Wire.endTransmission() == 0;
}

// ======================================================
// Read one AS5600
// ======================================================
bool readAS5600(uint8_t channel, uint16_t &rawAngle) {
  if (!selectMuxChannel(channel)) {
    return false;
  }

  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_RAW_ANGLE_REGISTER);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  uint8_t bytesReceived =
      Wire.requestFrom((uint8_t)AS5600_ADDRESS, (uint8_t)2);

  if (bytesReceived != 2) {
    return false;
  }

  uint8_t highByte = Wire.read();
  uint8_t lowByte = Wire.read();

  rawAngle = ((highByte & 0x0F) << 8) | lowByte;

  return true;
}

// ======================================================
// Convert raw reading to absolute 0–360 degrees
// ======================================================
float rawToAbsoluteDegrees(uint16_t rawAngle) {
  return rawAngle * 360.0f / 4096.0f;
}

// ======================================================
// Convert raw reading to signed calibrated angle
// ======================================================
float rawToCalibratedDegrees(
  uint16_t rawAngle,
  float zeroOffset
) {
  float currentAngle = rawToAbsoluteDegrees(rawAngle);
  float jointAngle = currentAngle - zeroOffset;

  // Prevent a jump when crossing the encoder's 0/360 point
  if (jointAngle > 180.0f) {
    jointAngle -= 360.0f;
  }

  if (jointAngle < -180.0f) {
    jointAngle += 360.0f;
  }

  return jointAngle;
}

// ======================================================
// Display the three-second countdown
// ======================================================
void displayCalibrationCountdown() {
  char countdownText[4];

  for (int seconds = 3; seconds >= 1; seconds--) {
    snprintf(
      countdownText,
      sizeof(countdownText),
      "%d",
      seconds
    );

    oled.clearBuffer();

    oled.setFont(u8g2_font_7x14B_tf);
    drawCenteredText(14, "ALIGN ARM");

    oled.setFont(u8g2_font_logisoso32_tn);
    drawCenteredText(52, countdownText);

    oled.sendBuffer();

    Serial.print("Calibrating in ");
    Serial.print(seconds);
    Serial.println("...");

    delay(1000);
  }
}

// ======================================================
// Record all seven zero positions
// ======================================================
bool calibrateLeaderArm(int &failedSensor) {
  float sineTotals[SENSOR_COUNT] = {0};
  float cosineTotals[SENSOR_COUNT] = {0};

  oled.clearBuffer();

  oled.setFont(u8g2_font_7x14B_tf);
  drawCenteredText(27, "HOLD STILL");

  oled.setFont(u8g2_font_6x12_tf);
  drawCenteredText(47, "CALIBRATING...");

  oled.sendBuffer();

  for (int sample = 0; sample < CALIBRATION_SAMPLES; sample++) {
    for (int i = 0; i < SENSOR_COUNT; i++) {
      uint16_t rawAngle;

      if (!readAS5600(sensorChannels[i], rawAngle)) {
        failedSensor = i;
        return false;
      }

      float angleRadians =
          rawAngle * TWO_PI / 4096.0f;

      sineTotals[i] += sinf(angleRadians);
      cosineTotals[i] += cosf(angleRadians);
    }

    delay(25);
  }

  // Calculate the average angle for each encoder
  for (int i = 0; i < SENSOR_COUNT; i++) {
    float averageRadians = atan2f(
      sineTotals[i],
      cosineTotals[i]
    );

    float averageDegrees =
        averageRadians * 180.0f / PI;

    if (averageDegrees < 0.0f) {
      averageDegrees += 360.0f;
    }

    zeroOffsetDegrees[i] = averageDegrees;

    Serial.print("Joint ");
    Serial.print(i + 1);
    Serial.print(" zero offset: ");
    Serial.print(zeroOffsetDegrees[i], 2);
    Serial.println(" degrees");
  }

  return true;
}

// ======================================================
// Display successful calibration
// ======================================================
void displayCalibrationComplete() {
  oled.clearBuffer();

  oled.setFont(u8g2_font_7x14B_tf);
  drawCenteredText(27, "CALIBRATION");

  oled.setFont(u8g2_font_6x12_tf);
  drawCenteredText(48, "COMPLETE");

  oled.sendBuffer();

  Serial.println("Calibration complete.");
  delay(1000);
}

// ======================================================
// Display failed calibration
// ======================================================
void displayCalibrationError(int failedSensor) {
  char errorText[24];

  snprintf(
    errorText,
    sizeof(errorText),
    "CHECK J%d ON CH%d",
    failedSensor + 1,
    sensorChannels[failedSensor]
  );

  oled.clearBuffer();

  oled.setFont(u8g2_font_7x14B_tf);
  drawCenteredText(22, "CAL FAILED");

  oled.setFont(u8g2_font_6x12_tf);
  drawCenteredText(42, errorText);
  drawCenteredText(58, "RESET TO RETRY");

  oled.sendBuffer();

  Serial.print("Calibration failed on Joint ");
  Serial.print(failedSensor + 1);
  Serial.print(", multiplexer channel ");
  Serial.println(sensorChannels[failedSensor]);
}

// ======================================================
// Display seven joints in individual boxes
// ======================================================
void displayReadings(float degrees[], bool sensorOK[]) {
  char readingText[16];

  oled.clearBuffer();
  oled.setDrawColor(1);
  oled.setFont(u8g2_font_6x10_tf);

  for (int i = 0; i < SENSOR_COUNT; i++) {
    int boxX;
    int boxY;
    int boxWidth;
    int boxHeight;

    // J1 through J6 use two columns
    if (i < 6) {
      int column = i % 2;
      int row = i / 2;

      boxX = column * 65;
      boxY = row * 16;
      boxWidth = 63;
      boxHeight = 15;
    }

    // J7 uses the full-width bottom box
    else {
      boxX = 0;
      boxY = 48;
      boxWidth = 128;
      boxHeight = 16;
    }

    oled.drawFrame(
      boxX,
      boxY,
      boxWidth,
      boxHeight
    );

    if (sensorOK[i]) {
      snprintf(
        readingText,
        sizeof(readingText),
        "J%d %5.1f",
        i + 1,
        degrees[i]
      );
    } else {
      snprintf(
        readingText,
        sizeof(readingText),
        "J%d ERROR",
        i + 1
      );
    }

    int textWidth = oled.getStrWidth(readingText);
    int textX = boxX + (boxWidth - textWidth) / 2;

    oled.drawStr(
      textX,
      boxY + 12,
      readingText
    );

    if (sensorOK[i]) {
      oled.drawCircle(
        textX + textWidth + 2,
        boxY + 4,
        1
      );
    }
  }

  oled.sendBuffer();
}

// ======================================================
// Setup
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(MUX_SDA, MUX_SCL, 100000);

  pinMode(OLED_RST, OUTPUT);

  digitalWrite(OLED_RST, LOW);
  delay(50);

  digitalWrite(OLED_RST, HIGH);
  delay(100);

  oled.setI2CAddress(0x3C << 1);
  oled.begin();
  oled.setPowerSave(0);
  oled.setContrast(255);

  displayCalibrationCountdown();

  int failedSensor = -1;

  if (!calibrateLeaderArm(failedSensor)) {
    displayCalibrationError(failedSensor);

    // Stop here until the ESP32 is reset
    while (true) {
      delay(1000);
    }
  }

  displayCalibrationComplete();
}

// ======================================================
// Main loop
// ======================================================
void loop() {
  uint16_t rawAngles[SENSOR_COUNT] = {0};
  float degrees[SENSOR_COUNT] = {0};
  bool sensorOK[SENSOR_COUNT] = {false};

  for (int i = 0; i < SENSOR_COUNT; i++) {
    sensorOK[i] = readAS5600(
      sensorChannels[i],
      rawAngles[i]
    );

    if (sensorOK[i]) {
      degrees[i] = rawToCalibratedDegrees(
        rawAngles[i],
        zeroOffsetDegrees[i]
      );
    }
  }

  displayReadings(degrees, sensorOK);

  for (int i = 0; i < SENSOR_COUNT; i++) {
    Serial.print("Joint ");
    Serial.print(i + 1);
    Serial.print(" (CH");
    Serial.print(sensorChannels[i]);
    Serial.print("): ");

    if (sensorOK[i]) {
      Serial.print(degrees[i], 1);
      Serial.print(" deg | Raw: ");
      Serial.print(rawAngles[i]);
    } else {
      Serial.print("SENSOR ERROR");
    }

    if (i < SENSOR_COUNT - 1) {
      Serial.print(" || ");
    }
  }

  Serial.println();
  delay(50);
}
