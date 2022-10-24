#include <WiFi.h>
#include <SolarCalculator.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>

const char* ssid = "ssid";
const char* password = "pass";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

// geolocation
const double latitude = 41.7;
const double longitude = 2.17;
// refresh interval, in seconds
int interval = 5;
// define data pin connected to the light
int pin = 4;
int num_pixels = 24;
// build Neopixel object
Adafruit_NeoPixel pixels(num_pixels, pin, NEO_GRB + NEO_KHZ800);
// global variables for RGB colors
int r, g, b;


// python equivalent split function
String split(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// get time function, given the time in a String format
void get_time(String data) {
  int year = split(data, ':', 0).toInt();
  int month = split(data, ':', 1).toInt();
  int day = split(data, ':', 2).toInt();
  int hour = split(data, ':', 3).toInt();
  int minute = split(data, ':', 4).toInt();
  int second = split(data, ':', 5).toInt();
  // set time manually (hour, minute, second, day, month, year)
  setTime(hour, minute, second, day, month, year);
}


float map_float(float val, float from_low, float from_high, float to_low, float to_high) {
  // slope
  float m = (to_high - to_low) / (from_high - from_low);
  // ecuation
  float y = to_low + m*(val - from_low);
  return y;
}


void get_light(float elevation) {
  /*
  * No Sun     [elevation -18] (5, 0, 3)
  * CCT 1000 K [elevation -10] (255, 30, 0)
  * CCT 1800 K [elevation 0]   (255, 70, 0)
  * CCT 2400 K [elevation 5]   (255, 100, 0)
  * CCT 3000 K [elevation 10]  (255, 120, 10)
  * CCT 3500 K [elevation 15]  (255, 130, 25)
  * CCT 4000 K [elevation 20]  (255, 140, 35)
  * CCT 5000 K [elevation 25]  (255, 160, 50)
  * CCT 6000 K [elevation 40]  (255, 160, 73)
  */
  // define variables to get the RGB values
  // number of datapoints
  int data_points = 9;
  // sun_elevation, r, g and b datapoints
  int sun_elevations[data_points] = {-18, -10, 0, 5, 10, 15, 20, 25, 40};
  int rs[data_points] = {5, 255, 255, 255, 255, 255, 255, 255, 255};
  int gs[data_points] = {0, 30, 70, 100, 120, 130, 140, 160, 160};
  int bs[data_points] = {3, 0, 0, 0, 10, 25, 35, 50, 73};
  // get the lowest datapoint closest to the elevation 
  int elevation_index = 0;
  for (int i = 0; i < data_points - 1; i++) {
    if (elevation > sun_elevations[i]) {
      elevation_index = i + 1;
    }
  }
  // if elevation if below the lowest datapoint
  if (elevation <= sun_elevations[0]) {
    r = rs[0];
    g = gs[0];
    b = bs[0];
  }
  else {
    // if elevation is somewhere in the middle
    if (elevation <= sun_elevations[elevation_index]) {
      // interpolate the RGB values
      r = map_float(elevation, sun_elevations[elevation_index-1], sun_elevations[elevation_index], rs[elevation_index-1], rs[elevation_index]);
      g = map_float(elevation, sun_elevations[elevation_index-1], sun_elevations[elevation_index], gs[elevation_index-1], gs[elevation_index]);
      b = map_float(elevation, sun_elevations[elevation_index-1], sun_elevations[elevation_index], bs[elevation_index-1], bs[elevation_index]);
    }
    // if elevation is above the highest datapoint
    else {
      r = rs[data_points-1];
      g = gs[data_points-1];
      b = bs[data_points-1];
    }
  }
}


// emit light function
void emit_light(float elevation) {
  get_light(elevation);
  // for every pixel
  for (int i = 0; i < num_pixels; i++) {
    // set RGB color (from 0 to 255)
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  // send the updated pixel colors to the hardware
  pixels.show();
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // initialize Neopixel object
  pixels.begin();
}


void loop() {
  // set all pixel colors to 0
  pixels.clear();
  
  // at every interval
  static unsigned long next_millis = 0;
  if (millis() > next_millis) {
    unsigned long utc = getTime();
    // define output variables
    double az, elevation;
    // calculate the solar position, in degrees
    calcHorizontalCoordinates(utc, latitude, longitude, az, elevation);
    // emit light
    emit_light(elevation);
    // time control
    next_millis = millis() + interval * 1000;
  }
}
