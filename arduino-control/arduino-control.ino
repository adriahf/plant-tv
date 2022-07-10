/*
  Plant TV Arduino Corntrol

  This sketch controls all the operating logic of the Plant TV project. 
 */

#include <Bridge.h>
#include <HttpClient.h>
#include <ArduinoJson.h>

void setup() {
  SerialUSB.begin(9600);
  // wait for a serial connection
  while (!SerialUSB); 
}

void loop() {
  String current_date_time = get_time_string();
  int year = current_date_time.substring(0, 4).toInt();
  int month = current_date_time.substring(5, 7).toInt();
  int day = current_date_time.substring(8, 10).toInt();
  int hour = current_date_time.substring(11, 13).toInt();
  int minute = current_date_time.substring(14, 16).toInt();
  
  SerialUSB.flush();

  delay(5000);
}
