#include "Arduino.h"
#include "myJSON.h"
// #include <ArduinoJson.h>
// #include "FS.h"



myJSON::myJSON() {
        // Serial.begin(9600);
        // if (!SPIFFS.begin()) {
        //         Serial.println("Failed to mount file system");
        // }
        yield;
}

void myJSON::ReadValue(){
  return 1;
        // File openFile = SPIFFS.open(filename, "r");
        // if (!openFile) {
        //         Serial.println("Failed to open config file");
        //         return false;
        // }
        //
        // size_t size = openFile.size();
        // if (size > 1024) {
        //         Serial.println("Config file size is too large");
        //         return false;
        // }
        //
        // // Allocate a buffer to store contents of the file.
        // std::unique_ptr<char[]> buf(new char[size]);
        //
        // // We don't use String here because ArduinoJson library requires the input
        // // buffer to be mutable. If you don't use ArduinoJson, you may as well
        // // use openFile.readString instead.
        // openFile.readBytes(buf.get(), size);
        //
        // StaticJsonBuffer<200> jsonBuffer;
        // JsonObject& json = jsonBuffer.parseObject(buf.get());
        //
        // if (!json.success()) {
        //         Serial.println("Failed to parse config file");
        //         return false;
        // }
        //
        // const char* key_1 = json[key];
        // const char *value = json[key];
        // // ret_value = value;
        //
        // // Real world application would store these values in some variables for
        // // later use.
        //
        // Serial.print(key);
        // Serial.print(": ");
        // Serial.println(value);
        // return true;
}

void myJSON::saveFile() {
  return 1;

        // StaticJsonBuffer<200> jsonBuffer;
        // JsonObject& json = jsonBuffer.createObject();
        // json[key] = value;
        //
        // File writeFile = SPIFFS.open(filename, "w");
        // if (!writeFile) {
        //         Serial.println("Failed to open config file for writing");
        //         return false;
        // }
        //
        // json.printTo(writeFile);
        // return true;
}
