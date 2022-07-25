#include <Process.h>
#include <SolarCalculator.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>


// process used to get the date
Process date;
// geolocation
const double latitude = 41.7;
const double longitude = 2.17;
int utc_offset = 2;
// refresh interval, in seconds
int interval = 5;
// define data pin connected to the light
int pin = 6;
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


// converts the time to UTC given a UTC offset
time_t to_utc(time_t local) {
  return local - long(utc_offset * 3600);
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
      r = map(elevation, sun_elevations[elevation_index-1], sun_elevations[elevation_index], rs[elevation_index-1], rs[elevation_index]);
      g = map(elevation, sun_elevations[elevation_index-1], sun_elevations[elevation_index], gs[elevation_index-1], gs[elevation_index]);
      b = map(elevation, sun_elevations[elevation_index-1], sun_elevations[elevation_index], bs[elevation_index-1], bs[elevation_index]);
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


void setup() {
  // initialize Neopixel object
  pixels.begin();
  // initialize Bridge
  Bridge.begin();
  // initialize serial 
  Serial.begin(9600); 
  // wait for Serial Monitor to open
  while(!Serial);
  Serial.println("Initializing...");
}


void loop() {
  // set all pixel colors to 0
  pixels.clear();
  static unsigned long next_millis = 0;
  // at every interval
  if (millis() > next_millis) {
    // begin date
    date.begin("/bin/date");
    // time format
    date.addParameter("+%Y:%m:%d:%H:%M:%S");
    date.run();
    // if there's a result from the date process, get it.
    while (date.available() > 0) {
      // get the time
      get_time(date.readString());
      // get time in UNIX timestamp
      time_t utc = to_utc(now());
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
}
