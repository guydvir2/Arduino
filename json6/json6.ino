#include <ArduinoJson.h>
#include "FS.h"
#define filename "file.json"


StaticJsonDocument<200> doc;

void createJSON() {
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;

  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  // Generate the minified JSON and send it to the Serial port.
  serializeJson(doc, Serial);
  Serial.println("JSON is created:");

  // Generate the prettified JSON and send it to the Serial port.
  serializeJsonPretty(doc, Serial);

}



void saveJSON2file() {
  File writeFile = SPIFFS.open(filename, "w");
  serializeJson(doc, writeFile);

  Serial.println("JSON file is saved");
}

void readJSON_file(){
  StaticJsonDocument<512> read_doc;
  char json [100];
  File readFile = SPIFFS.open(filename, "r");
  DeserializationError error = deserializeJson(read_doc, readFile);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  serializeJson(read_doc, Serial);

  serializeJson(read_doc, json);
  Serial.print("from file:");
  Serial.println(json);

}
void setup() {
  Serial.begin(9600);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }
//  createJSON();
//  saveJSON2file();
  readJSON_file();
}

void loop() {
  // not used in this example
}
