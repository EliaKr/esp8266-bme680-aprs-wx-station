#define CALLSIGN "YOUR_CALLSIGN-13" // your callsign and SSID (usually -13)
#define PASSCODE "YOUR_PASSCODE" // your APRS-IS passcode
#define LAT "YOUR_LATITUDE" // formatted for APRS (you can use https://aprs.wiki/converter/ to convert your coordinates)
#define LONG "YOUR_LONGITUDE" // formatted for APRS (you can use https://aprs.wiki/converter/ to convert your coordinates)
#define MESSAGE "YOUR_MESSAGE" // the message contained in the APRS packet

#define WIFI_SSID "YOUR_SSID" // your WiFi SSID
#define WIFI_PASS "YOUR_WIFI_PASSWORD" // your WiFi password
#define SERVER "rotate.aprs2.net" // APRS-IS server
#define PORT 14580 // APRS-IS server port
#define DELAY 300000 // delay between measurements in milliseconds

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h> // change the i2c address in the definition of the Adafruit_BME680 object if necessary. Default is 0x77

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

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
}

void loop() {
  Measurements measurements = getMeasurements();
  String message = createAPRSMessage(measurements);
  sendAPRSMessage(message);

  // comment out the correct line dependiong on whether you want to sleep or not
  // delay(DELAY);
  long deepSleepTime = DELAY * 1000;
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

// formats the measurements and data to create the APRS message
String createAPRSMessage(Measurements measurements){
  String message = CALLSIGN;
  
  message += ">APRS,TCPIP*:";
  message += "@......z";
  message += LAT;
  message += "/";
  message += LONG;
  message += "_.../...g...";

  message += "t"; // Temperature prefix

  float temperatureF = (measurements.temperature * 9.0 / 5.0) + 32.0;
  char temperatureStr[4];
  snprintf(temperatureStr, sizeof(temperatureStr), "%03d", (int)temperatureF);
  message += temperatureStr; // temperature in Fahrenheit

  message += "h"; // Humidity prefix
  char humidityStr[3];
  snprintf(humidityStr, sizeof(humidityStr), "%02d", (int)measurements.humidity);
  message += humidityStr; // humidity value 2 digits

  message += "r...p...P...";

  message += "b";            // Pressure prefix
  char pressureStr[6];
  snprintf(pressureStr, sizeof(pressureStr), "%05d", (int)(measurements.pressure * 10));
  message += pressureStr; // Pressure value 5 digits

  message += " ";
  message += MESSAGE;

  message += " [Gas resistance: ";
  char gasResistanceStr[10];
  snprintf(gasResistanceStr, sizeof(gasResistanceStr), "%.2f", measurements.gasResistance);
  message += gasResistanceStr;
  message += "kΩ]";

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
