#include <Wire.h>
#include <U8g2lib.h>
#include <MPU6050_tockn.h>
#include <DHT.h>
#include <BluetoothSerial.h>
#include <math.h>

/*
SAME CONNECTIONS, SAME DEVICE, SAME MIRRORING
TTP223 on GPIO 25 as HUD ON/OFF switch
*/

// ---------- Bluetooth ----------
BluetoothSerial SerialBT;
bool btConnected = false;
String timeStr = "--:--";

// ---------- TTP223 ----------
#define TTP_PIN 25
bool hudOn = true;
bool lastTouch = LOW;

// ---------- DHT11 ----------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- OLED ----------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_MIRROR,
  U8X8_PIN_NONE,
  U8X8_PIN_NONE,
  U8X8_PIN_NONE
);

// ---------- MPU6050 ----------
TwoWire I2C_MPU(1);
MPU6050 mpu(I2C_MPU);

// ---------- Dummy flight data ----------
float airspeed = 850;
float altitude = 38500;
float heading  = 90;

void setup() {
  pinMode(TTP_PIN, INPUT);

  // OLED
  Wire.begin(18, 19, 400000);
  u8g2.begin();
  u8g2.setFont(u8g2_font_5x7_mf);

  // MPU
  I2C_MPU.begin(21, 22, 400000);
  mpu.begin();
  mpu.calcGyroOffsets(true);

  // DHT
  dht.begin();

  // Bluetooth
  SerialBT.begin("Spectra-HUD");
}

// ---------- Drawing ----------
void drawPitchLadderSimple(float pitch) {
  int cx = 64, cy = 32;

  for (int p = -20; p <= 20; p += 5) {
    int y = cy + (p - pitch) * 2;
    int len = (p % 10 == 0) ? 20 : 10;

    u8g2.drawLine(cx - len, y, cx + len, y);

    if (p % 10 == 0) {
      u8g2.setCursor(cx + len + 2, y + 3);
      u8g2.print(abs(p));
    }
  }
}

// ✈️ Fixed position, tilting horizon
void drawTiltedHorizon(float roll) {
  int cx = 64, cy = 32;
  int length = 120;

  float rad = roll * DEG_TO_RAD;

  int x1 = cx - (length / 2) * cos(rad);
  int y1 = cy - (length / 2) * sin(rad);
  int x2 = cx + (length / 2) * cos(rad);
  int y2 = cy + (length / 2) * sin(rad);

  u8g2.drawLine(x1, y1, x2, y2);
}

// 🔴 Pitch-based moving circle
void drawMovingCircle(float pitch) {
  int cx = 64, cy = 32;
  int y = cy - pitch * 2;
  y = constrain(y, 8, 56);
  u8g2.drawCircle(cx, y, 6);
}

// 📶 Bluetooth icon
void drawBluetoothIcon() {
  if (!btConnected) return;

  int x = 118;
  int y = 6;

  u8g2.drawLine(x, y, x, y + 10);
  u8g2.drawLine(x, y, x - 4, y + 4);
  u8g2.drawLine(x, y + 10, x - 4, y + 6);
}

// ⏰ Time display (left-middle)
void drawTime() {
  u8g2.setCursor(0, 36);
  u8g2.print(timeStr);
}

void drawSpeed(float s) {
  u8g2.setCursor(0, 10);
  u8g2.print((int)s);
  u8g2.print("km/h");
}

void drawAltitude(float a) {
  u8g2.setCursor(90, 10);
  u8g2.print((int)a);
  u8g2.print("ft");
}

void drawHeading(float h) {
  u8g2.setCursor(48, 63);
  u8g2.print("HDG ");
  u8g2.print((int)h);
}

void drawTemperatureHumidity(float t, float h) {
  u8g2.setCursor(0, 20);
  u8g2.print((int)t);
  u8g2.print("C");

  u8g2.setCursor(100, 20);
  u8g2.print((int)h);
  u8g2.print("%");
}

void loop() {
  // ---------- Bluetooth ----------
  btConnected = SerialBT.hasClient();

  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    data.trim();

    if (data.startsWith("TIME:")) {
      timeStr = data.substring(5);
    }
  }

  // ---------- Touch Toggle ----------
  bool touch = digitalRead(TTP_PIN);
  if (touch == HIGH && lastTouch == LOW) {
    hudOn = !hudOn;
    delay(300);
  }
  lastTouch = touch;

  if (!hudOn) {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    return;
  }

  // ---------- Sensors ----------
  mpu.update();
  float pitch = -mpu.getAngleX();
  float roll  =  mpu.getAngleY();
  float yaw   =  mpu.getAngleZ();
  heading = yaw;

  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // ---------- Draw ----------
  u8g2.clearBuffer();

  drawPitchLadderSimple(pitch);
  drawTiltedHorizon(roll);
  drawMovingCircle(pitch);
  drawSpeed(airspeed);
  drawAltitude(altitude);
  drawHeading(heading);
  drawTime();
  drawBluetoothIcon();

  if (!isnan(temperature) && !isnan(humidity)) {
    drawTemperatureHumidity(temperature, humidity);
  }

  u8g2.sendBuffer();
}
