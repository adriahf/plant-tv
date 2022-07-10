String get_time_request() {
  // Initialize the client library
  HttpClient client;

  // Make a HTTP request:
  client.get("http://worldclockapi.com/api/json/utc/now");
  //client.get("http://www.arduino.org");
  int line_length = 235;
  char line[line_length];
  int index = 0;
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    if (index < line_length) {
      line[index] = c;
      index++;
    }
  }
  return line;
}

String parse_json_date_time(String line) {
  // Allocate JSON document in 384 bytes (recommeneded by the assistant)
  StaticJsonDocument<384> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, line);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  String current_date_time = doc["currentDateTime"];
  return current_date_time;
}

String get_time_string() {
  String time_request = get_time_request();
  String current_date_time = parse_json_date_time(time_request);
  return current_date_time;
}
