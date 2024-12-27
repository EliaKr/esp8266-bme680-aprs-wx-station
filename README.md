# esp8266-bme680-aprs-wx-station
A simple and lightweight APRS weather station using the BME680 sensor and ESP8266.

## Hardware Requirements
- ESP8266 WiFi enabled microcontroller
- BME680 environmental sensor

## Software Dependencies
### Libraries:
- ESP8266WiFi
- Wire
- Adafruit_Sensor

## Setup
### Wiring:
- Connect the BME680 sensor to the i2c pins on the ESP8266 (SCL -> D1, SDA -> D2)
- Connect the GND and VCC pins to ground and the correct voltage output on the ESP depending on your sensor model
- Connect D0 (GPIO16/WAKE) to RST to use deep sleep. (Enabled by default, to disable it comment out lines 82-83 and uncomment line 81.

### Software Configuration
- Replace all the information in the initial defines with your information

## Usage
- Connect the ESP8266 to external power and the station will start monitoring
