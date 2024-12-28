// A c++ script that creates an APRS message containing sample data.

#define CALLSIGN "YOUR_CALLSIGN" // your callsign and SSID (usually -13)
#define PASSCODE "YOUR_PASSCODE" // your APRS-IS passcode
#define LAT "YOUR_LATITUDE" // formatted for APRS (you can use https://aprs.wiki/converter/ to convert your coordinates)
#define LONG "YOUR_LONGITUDE" // formatted for APRS (you can use https://aprs.wiki/converter/ to convert your coordinates)
#define MESSAGE "YOUR_MESSAGE" // the message contained in the APRS packet

#define SERVER "http://rotate.aprs2.net" // APRS-IS server
#define PORT 14580 // APRS-IS server port
#define DELAY 300000 // delay between measurements in milliseconds

#define TEMPERATURE 0.0 // sample temperature
#define HUMIDITY 0.0 // sample humidity
#define PRESSURE 1000.0 // sample pressure
#define GAS 00000 // sample gas resistance

#include <iostream>
#include <string>

using namespace std;

struct Measurements {
  float temperature;
  float pressure;
  float humidity;
  float gasResistance;
};

Measurements getMeasurements();
string createAPRSMessage(Measurements measurements);

int main() {
    printf("Starting APRS message test\n");
    Measurements measurements;
    measurements = getMeasurements();
    string message;
    message = createAPRSMessage(measurements);
    printf("APRS message: %s\n", message.c_str());

    return 0;
}

// returns the measurements from the BME680 sensor
Measurements getMeasurements() {
  Measurements measurements;

  // Store sensor data in the measurements struct
  measurements.temperature = TEMPERATURE;
  measurements.pressure = PRESSURE;
  measurements.humidity = HUMIDITY;
  measurements.gasResistance = GAS / 1000.0;

  return measurements;
}

string createAPRSMessage(Measurements measurements){
  string message = CALLSIGN;
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
