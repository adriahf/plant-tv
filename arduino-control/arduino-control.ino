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


// emit light function
void emit_light(float elevation) {
  /*
   * No Sun     (5, 0, 3)
   * CCT 1000 K (255, 30, 0)
   * CCT 1800 K (255, 70, 0)
   * CCT 2400 K (255, 100, 0)
   * CCT 3000 K (255, 120, 10)
   * CCT 3500 K (255, 130, 25)
   * CCT 4000 K (255, 140, 35)
   * CCT 5000 K (255, 160, 50)
   * CCT 6000 K (255, 160, 73)
   */
  // for every pixel
  for (int i=0; i<num_pixels; i++) {
    // set RGB color (from 0 to 255)
    pixels.setPixelColor(i, pixels.Color(255, 160, 73));
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
      Serial.println(elevation);
      // emit light
      emit_light(elevation);
      // time control
      next_millis = millis() + interval * 1000;
    }
  }
}
