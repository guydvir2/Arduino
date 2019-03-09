#include "Arduino.h"
#include "myJSON.h"

#include <ArduinoJson.h>
#include "FS.h"



myJSON::myJSON() {
  if (!SPIFFS.begin()) {
   Serial.println("Failed to mount file system");
}
void myIOT::start_services(char *filename) {
        mqtt_server = mqtt_broker;
        user = mqtt_user;
        passw = mqtt_passw;
        ssid = ssid;
        password = password;
        ext_mqtt = funct;   //redirecting to ex-class function ( defined outside)
        extDefine = true;   // maing sure this ext_func was defined


        if ( useSerial ) {
                Serial.begin(9600);
                delay(10);
        }
        startNetwork(ssid, password);
        if (useWDT) {
                startWDT();
        }
        if (useOTA) {
                startOTA();
        }
}

bool myJSON::ReadValue(char *filename, char *key){
  File openFile = SPIFFS.open(filename, "r");
  if (!openFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = openFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use openFile.readString instead.
  openFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  const char* key_1 = json[key];
  const char *value = json[key];

  // Real world application would store these values in some variables for
  // later use.

  Serial.print(key);
  Serial.print(": ");
  Serial.println(value);
  return true;                           bb5

}
