#include "Arduino.h"
#include "myJSON.h"
#include "FS.h"
#include <ArduinoJson.h>

#define LOG_LENGTH 4

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
        delay(50);
        // Serial.println("JSON file saved OK");
        // myJSON::PrettyprintJSON(_doc);
}
void myJSON::readJSON_file(JsonDocument& _doc) {
        File readFile = SPIFFS.open(_filename, "r");
        DeserializationError error = deserializeJson(_doc, readFile);
        if (error) {
                Serial.println(F("Failed to read file"));
        }
        else{
                serializeJson(_doc, readFile);
        }
        delay(50);
}
void myJSON::printJSON(JsonDocument& _doc) {
        serializeJson(_doc, Serial);
}
void myJSON::PrettyprintJSON(JsonDocument& _doc) {
        serializeJsonPretty(_doc, Serial);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~ User Functions : JSON + file saving ~~~~~~~~~~~
bool myJSON::getValue (const char *key, char value[20]){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey) {
                const char *val = tempJDOC[key];
                sprintf(value,"%s",val);
                return 1;
        }
        else {
                return 0; // when key is not present
        }
}
bool myJSON::getValue (const char *key, int &retval){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey) {
                retval = tempJDOC[key];
                return 1;
        }
        else {
                return 0; // when key is not present
        }
}
bool myJSON::getValue (const char *key, long &retval){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey) {
                retval = tempJDOC[key];
                return 1;
        }
        else {
                return 0; // when key is not present
        }
}

void myJSON::setValue(const char *key, char *value){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        tempJDOC[key]=value;
        myJSON::saveJSON2file(tempJDOC);
}
void myJSON::setValue(const char *key, int value){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        tempJDOC[key]=value;
        myJSON::saveJSON2file(tempJDOC);
        myJSON::PrettyprintJSON(tempJDOC);
}
void myJSON::setValue(const char *key, long value){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        tempJDOC[key]=value;
        myJSON::saveJSON2file(tempJDOC);
}

void myJSON::updateArray(char* array_key, char *val) {
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull()) { // create for the first time
                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH) {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH) {
                for (int n = 0; n < LOG_LENGTH - 1; n++) {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        myJSON::saveJSON2file(tempJDOC);
}
void myJSON::updateArray(char* array_key, int val) {
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull()) { // create for the first time
                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH) {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH) {
                for (int n = 0; n < LOG_LENGTH - 1; n++) {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        myJSON::saveJSON2file(tempJDOC);
}
void myJSON::updateArray(char* array_key, long val) {
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull()) { // create for the first time
                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH) {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH) {
                for (int n = 0; n < LOG_LENGTH - 1; n++) {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        myJSON::saveJSON2file(tempJDOC);
}

void myJSON::nestedArray(char* array_key, long val) {
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        JsonArray array = tempJDOC.to<JsonArray>();
        JsonObject nested = array.createNestedObject();
        nested[array_key] = val;
        myJSON::saveJSON2file(tempJDOC);
}

void myJSON::eraseArray(char* array_key) {
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        JsonArray data = tempJDOC.createNestedArray(array_key);
        myJSON::saveJSON2file(tempJDOC);
}
void myJSON::removeValue(const char *key){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        tempJDOC.remove(key);
        myJSON::saveJSON2file(tempJDOC);
}

void myJSON::printFile(){
        StaticJsonDocument<DOC_SIZE> tempJDOC;
        myJSON::readJSON_file(tempJDOC);
        serializeJsonPretty(tempJDOC, Serial);


}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
