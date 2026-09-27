# Aviation HUD Helmet

A flight helmet with a real heads-up display. An SSD1306 draws a mirrored attitude display that a beam-splitter prism floats in the visor. This repository holds the ESP32 display firmware (phase 5A). The full helmet build, with a servo-actuated visor, integrated audio and BLE turn-by-turn directions from the phone, is in development.

**Videos:** [HUD helmet build](https://youtu.be/bDn9z9ZJ6eI) · [helmet short](https://youtube.com/shorts/g4ehRAIy6fM)

## What the firmware draws

- **Pitch ladder** with 5° steps, labelled every 10°, driven by the MPU6050
- **Tilting horizon** that follows roll, with a fixed aircraft reference
- **Pitch marker**, a circle that rides up and down with pitch
- **Heading** from the gyro's yaw (`HDG`)
- **Time** pushed from the phone over Bluetooth, plus a Bluetooth icon while a phone is connected
- **Temperature and humidity** from a DHT11
- **Speed and altitude placeholders** (fixed demo values in this phase)

The whole frame is rendered with `U8G2_MIRROR`, so it reads correctly after the reflection in the beam-splitter. A TTP223 touch pad toggles the HUD on and off.

## Hardware

| Part | Role |
|---|---|
| ESP32 (WROOM-32) | Controller, Bluetooth Classic |
| 0.96" 128 × 64 SSD1306 OLED (I²C) | HUD source image |
| 25 mm beam-splitter prism | Floats the image in the visor |
| MPU6050 | Pitch, roll and yaw |
| DHT11 | Temperature and humidity |
| TTP223 | Capacitive HUD on/off |

**Two I²C buses**, so the display and the IMU never contend:

| Bus | Device | SDA | SCL |
|---|---|---|---|
| `Wire` (400 kHz) | SSD1306 | 18 | 19 |
| `I2C_MPU` (400 kHz) | MPU6050 | 21 | 22 |

The DHT11 data pin goes to GPIO 4 and the TTP223 output to GPIO 25.

## Build and flash

1. Install the ESP32 board package in the Arduino IDE.
2. Install `U8g2`, `MPU6050_tockn` and `DHT sensor library` (Adafruit).
3. Open `phase_5A.ino`, select your ESP32 board and upload. Keep the helmet still for the first few seconds while the gyro offsets calibrate.
4. Pair with `Spectra-HUD` over Bluetooth and send `TIME:hh:mm` to set the clock.

More builds: [portfolio](https://portfolio-mridul-six.vercel.app) · [YouTube](https://www.youtube.com/@mridulsharma-martian)

## License

MIT. See [LICENSE](LICENSE).
