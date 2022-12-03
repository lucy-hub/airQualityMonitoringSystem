/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/lucy/Mr_argon/ArgonAirQualitySystem/airQualityMonitoringSystem/src/airQualityMonitoringSystem.ino"
/*
 * Project airQualityMonitoringSystem
 * Description: :)
 * Author: Lucy Smith
 * Date: 02/12/22
 */

#include <math.h>
#include "Air_Quality_Sensor.h"
#include "Adafruit_BME280.h"
#include "SeeedOLED.h"
#include "JsonParserGeneratorRK.h"

void getDustSensorReadings();
String getAirQuality();
void createEventPayload(int temp, int humidity, int pressure, String airQuality);
int getBMEValues(int &temp, int &pressure, int &humidity);
void updateDisplay(int temp, int humidity, int pressure, String airQuality);
void setup();
void loop();
#line 14 "/Users/lucy/Mr_argon/ArgonAirQualitySystem/airQualityMonitoringSystem/src/airQualityMonitoringSystem.ino"
#define DUST_SENSOR_PIN D4
#define SENSOR_READING_INTERVAL 30000
#define AQS_PIN A2


AirQualitySensor aqSensor(AQS_PIN);

unsigned long lastInterval;
unsigned long lowpulseoccupancy = 0;
unsigned long last_lpo = 0;
unsigned long duration;

float ratio = 0;
float concentration = 0;

Adafruit_BME280 bme;

void getDustSensorReadings()
{
  if (lowpulseoccupancy == 0){
 lowpulseoccupancy = last_lpo;
}
else{
 last_lpo = lowpulseoccupancy;
}

ratio = lowpulseoccupancy / (SENSOR_READING_INTERVAL * 10.0);
concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;



Serial.printlnf("LPO: %lu", lowpulseoccupancy);
Serial.printlnf("Ratio: %f%%", ratio);
Serial.printlnf("Concentration: %f pcs/L", concentration);
}






String getAirQuality()
{
 int quality = aqSensor.slope();
 String qual = "None";

 if (quality == AirQualitySensor::FORCE_SIGNAL)
 {
   qual = "Danger";
 }
 else if (quality == AirQualitySensor::HIGH_POLLUTION)
 {
   qual = "High Pollution";
 }
 else if (quality == AirQualitySensor::LOW_POLLUTION)
 {
   qual = "Low Pollution";
 }
 else if (quality == AirQualitySensor::FRESH_AIR)
 {
   qual = "Fresh Air";
 }

 return qual;
}
int led = D7;




void createEventPayload(int temp, int humidity, int pressure, String airQuality)
{
  JsonWriterStatic<256> jw;
  {
    JsonWriterAutoObject obj(&jw);

    jw.insertKeyValue("temp", temp);
    jw.insertKeyValue("humidity", humidity);
    jw.insertKeyValue("pressure", pressure);
    jw.insertKeyValue("air-quality", airQuality);

    if (lowpulseoccupancy > 0)
    {
      jw.insertKeyValue("dust-lpo", lowpulseoccupancy);
      jw.insertKeyValue("dust-ratio", ratio);
      jw.insertKeyValue("dust-concentration", concentration);
    }
  }

  Particle.publish("env-vals", jw.getBuffer(), PRIVATE);
}




int getBMEValues(int &temp, int &pressure, int &humidity)
{
 temp = (int)bme.readTemperature();
 pressure = (int)(bme.readPressure() / 100.0F);
 humidity = (int)bme.readHumidity();

 return 1;
}

void updateDisplay(int temp, int humidity, int pressure, String airQuality)
{
 SeeedOled.clearDisplay();

 SeeedOled.setTextXY(0, 3);
 SeeedOled.putString(airQuality);

 SeeedOled.setTextXY(2, 0);
 SeeedOled.putString("Temp: ");
 SeeedOled.putNumber(temp);
 SeeedOled.putString("C");

 SeeedOled.setTextXY(3, 0);
 SeeedOled.putString("Humidity: ");
 SeeedOled.putNumber(humidity);
 SeeedOled.putString("%");

 SeeedOled.setTextXY(4, 0);
 SeeedOled.putString("Hello Thomas");
 //SeeedOled.putString("Press: ");
 //SeeedOled.putNumber(pressure);
 //SeeedOled.putString(" hPa");

 if (concentration > 1)
 {
   SeeedOled.setTextXY(5, 0);
   SeeedOled.putString("Dust: ");
   SeeedOled.putNumber(concentration); // Cast our float to an int to make it more compact
   SeeedOled.putString(" pcs/L");
 }
}



// setup() runs once, when the device is first turned on.
void setup() {

  //Dust detector setup
  Serial.begin(9600);

  pinMode(DUST_SENSOR_PIN, INPUT);
  lastInterval = millis();  
  
  pinMode(led, OUTPUT);

  //Air Quality Sensor setup
  if (aqSensor.init())
 {
   Serial.println("Air Quality Sensor ready.");
 }
 else
 {
   Serial.println("Air Quality Sensor ERROR!");
 }


  //Measuring temperature, humidity and pressure
  if (bme.begin()){
    Serial.println("BME280 Sensor ready.");
  }
  else{
    Serial.println("BME280 Sensor ERROR!");
  }


  //Grove OLED device setup
  Wire.begin();
  SeeedOled.init();

  SeeedOled.clearDisplay();
  SeeedOled.setNormalDisplay();
  SeeedOled.setPageMode();

  SeeedOled.setTextXY(2, 0);
  SeeedOled.putString("Particle");
  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString("Air Quality");
  SeeedOled.setTextXY(4, 0);
  SeeedOled.putString("Monitor");

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  

  //flashing light
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);

  //Dust detector loop
  duration = pulseIn(DUST_SENSOR_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;

  if ((millis() - lastInterval) > SENSOR_READING_INTERVAL)
  {
    getDustSensorReadings();

    lowpulseoccupancy = 0;
    lastInterval = millis();
  }

  //Air quality sensor loop
  String quality = getAirQuality();
  Serial.printlnf("Air Quality: %s", quality.c_str());

  //Measuring temperature, humidity and pressure loop

  int temp, pressure, humidity;

  getBMEValues(temp, pressure, humidity);
  Serial.printlnf("Temp: %d", temp);
  Serial.printlnf("Pressure: %d", pressure);
  Serial.printlnf("Humidity: %d", humidity);


  //Display loop
  updateDisplay(temp, humidity, pressure, quality);

  //Publish data
  createEventPayload(temp, humidity, pressure, quality);
}