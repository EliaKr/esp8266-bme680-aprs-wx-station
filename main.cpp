#define CALLSIGN "YOUR_CALLSIGN-13" // your callsign and SSID (usually -13)
#define PASSCODE "YOUR_PASSCODE" // your APRS-IS passcode
#define LAT "YOUR_LATITUDE" // formatted for APRS (you can use https://aprs.wiki/converter/ to convert your coordinates)
#define LONG "YOUR_LONGITUDE" // formatted for APRS (you can use https://aprs.wiki/converter/ to convert your coordinates)
#define ALTITUDE "YOUR_ALTITUDE" // altitude in meters. Intended for static weather reporting
#define MESSAGE "YOUR_MESSAGE" // the message contained in the APRS packet

#define SSID "YOUR_SSID" // your WiFi SSID
#define WIFIPASS "YOUR_WIFI_PASSWORD" // your WiFi password
#define SERVER "rotate.aprs2.net" // APRS-IS server
#define PORT 14580 // APRS-IS server port
#define DELAY 300000 // delay between measurements in milliseconds

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h> // change the i2c address in the definition of the Adafruit_BME680 object if necessary. Default is 0x77

const char* ssid = SSID;
const char* password = WIFIPASS;

struct Measurements {
  float temperature;
  float pressure;
  float humidity;
  float gasResistance;
};

Adafruit_BME680 bme; // I2C (connect to the I2C pins on the ESP8266, pins D1 SCL and D2 SDA)

Measurements getMeasurements();
String createAPRSMessage(Measurements measurements);
void sendAPRSMessage(String message);

void setup() {
  Serial.begin(115200);
  delay(10);

  // BME680 I2C
  // Initialize the BME680 sensor
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // NETWORK
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // APRS
  // Connect to the APRS-IS server
}

void loop() {
  Measurements measurements = getMeasurements();
  String message = createAPRSMessage(measurements);
  sendAPRSMessage(message);

  // comment out the correct line dependiong on whether you want to sleep or not
  // delay(DELAY);
  int deepSleepTime = DELAY * 1000000;
  ESP.deepSleep(deepSleepTime); // sleep for DELAY seconds (useful for battery powered devices). Needs GPIO16(D0) connected to RST.
}

// returns the measurements from the BME680 sensor
Measurements getMeasurements() {
  Measurements measurements;

  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println("Failed to begin reading :(");
    return measurements;
  }

  // Wait for the measurement to complete.
  delay(endTime - millis());

  if (!bme.endReading()) {
    Serial.println("Failed to complete reading :(");
    return measurements;
  }

  // Store sensor data in the measurements struct
  measurements.temperature = bme.temperature; // temperature in celsius
  measurements.pressure = bme.pressure / 100.0; // pressure in hpa
  measurements.humidity = bme.humidity; // float between 0-100
  measurements.gasResistance = bme.gas_resistance / 1000.0; // Gas resistance in kΩ

  return measurements;
}

// sample message CALLSIGN>APRS,TCPIP*:3752.49N/12225.16W_t22.5Ch65%p1013.2hPa Weather station report.
// formats the measurements and data to create the APRS message
String createAPRSMessage(Measurements measurements){
  String message = CALLSIGN;
  message += ">APRS,TCPIP*:";
  message += LAT;
  message += "/";
  message += LONG;
  message += "_t";
  message += measurements.temperature;
  message += "C";
  message += "h";
  message += measurements.humidity;
  message += "%";
  message += "p";
  message += measurements.pressure;
  message += "hPa";
  message += "a";            // Altitude prefix
  message += ALTITUDE; // Altitude value
  message += "m";            // Altitude unit (meters)
  message += " ";
  message += MESSAGE;
  message += " ";
  message += "[Gas resistance: ";
  message += measurements.gasResistance;
  message += "kΩ]";
  message += " #";  // This represents the weather station icon in APRS

  return message;
}

void sendAPRSMessage(String message){
  WiFiClient client;
  const char* host = SERVER;
  const int port = PORT;

  if (!client.connect(host, port)) {
    Serial.println("Connection to APRS-IS server failed");
    return;
  }

  Serial.println("Connected to APRS-IS server");

  // Send login string
  String loginString = "user " + String(CALLSIGN) + " pass " + String(PASSCODE) + " vers ESP8266WeatherStation";
  client.println(loginString);
  Serial.println("Sent login string: " + loginString);

  // Wait for server response
  while (client.connected() && !client.available()) {
    delay(100);
  }

  // Read server response
  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  // Send APRS message
  client.println(message);
  Serial.println("Sent APRS message: " + message);

  // Wait for server response
  while (client.connected() && !client.available()) {
    delay(100);
  }

  // Read server response
  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  client.stop();
}
