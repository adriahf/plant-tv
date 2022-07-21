#include <Process.h>

// process used to get the date
Process date;

// Python equivalent split function
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
}

void setup() {
  // initialize Bridge
  Bridge.begin();
  // initialize serial 
  Serial.begin(9600); 
  // wait for Serial Monitor to open
  while(!Serial);
  Serial.println("Initializing...");
}

void loop() {
  // begin date
  date.begin("/bin/date");
  // time format
  date.addParameter("+%Y:%m:%d:%H:%M:%S");
  date.run();
  
  // if there's a result from the date process, get it.
  while (date.available() > 0) {
    // get the time
    get_time(date.readString());
  }
  
  delay(5000); // wait 5000ms
}
