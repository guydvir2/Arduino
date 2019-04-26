#include "Arduino.h"
#include "myJSON.h"
#include "FS.h"
#include <ArduinoJson.h>


myJSON::myJSON(char *filename, bool useserial) {
        _useSerial = useserial;
        sprintf(_filename,"%s",filename);
        if (_useSerial) {
                Serial.begin(9600);
        }
        if (!SPIFFS.begin()) {
                if (_useSerial) {
                        Serial.println("Failed to mount file system");
                }
        }
}


// ~~~~~~~~~~~~~~ File Functions ~~~~~~~~~~~
bool myJSON::file_exists(){
        if(SPIFFS.begin()) {
                return SPIFFS.exists(_filename);
        }
}
bool myJSON::file_remove(){
        if(SPIFFS.begin()) {
                return SPIFFS.remove(_filename);
        }
}
bool myJSON::format(){
        if(SPIFFS.begin()) {
                if (_useSerial) {
                        Serial.print("Formating...");
                }
                bool flag = SPIFFS.format();
                if (_useSerial) {
                        if (flag) {
                                Serial.println("Done");
                        }
                        else {
                                Serial.println("Failed");
                        }
                }
                return flag;
        }
}
bool myJSON::FS_ok(){
        return SPIFFS.begin();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~ JSON Functions ~~~~~~~~~~~
void myJSON::saveJSON2file(JsonDocument& _doc) {
        File writeFile = SPIFFS.open(_filename, "w");
        serializeJson(_doc, writeFile);
        Serial.println("JSON file saved OK");
}
void myJSON::readJSON_file(JsonDocument& _doc) {
        File readFile = SPIFFS.open(_filename, "r");
        DeserializationError error = deserializeJson(_doc, readFile);
        if (error) {
                Serial.println(F("Failed to read file"));
        }
        else{
                serializeJson(_doc, readFile);
                Serial.println("JSON file read OK");
        }
}
void myJSON::printJSON(JsonDocument& _doc) {
        serializeJson(_doc, Serial);
}
void myJSON::PrettyprintJSON(JsonDocument& _doc) {
        serializeJsonPretty(_doc, Serial);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
