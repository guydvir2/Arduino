#include "Arduino.h"
#include "myJSON.h"
#include "FS.h"
#include <ArduinoJson.h>


myJSON::myJSON(char *filename, bool useserial) {
        useSerial = useserial;

        if (useSerial) {
                Serial.begin(9600);
        }
        if (!SPIFFS.begin()) {
                if (useSerial) {
                        Serial.println("Failed to mount file system");
                }
        }
        else{
                sprintf(_filename,"%s",filename);
        }
}

bool myJSON::ReadVal(char* key, char *ret_value){
        File openFile = SPIFFS.open(_filename, "r");
        if (!openFile) {
                if (useSerial) {
                        Serial.println("Failed to open config file");
                }
                return false;
        }
        size_t size = openFile.size();
        if (size > 1024) {
                if (useSerial) {
                        Serial.println("Config file size is too large");
                }
                return false;
        }

        std::unique_ptr<char[]> buf(new char[size]);
        openFile.readBytes(buf.get(), size);

        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());

        if (!json.success()) {
                if (useSerial) {
                        Serial.println("Failed to parse config file");
                }
                return false;
        }

        const char *value = json[key];
        sprintf(ret_value,"%s",value);
        return true;
}

bool myJSON::SaveVal(char *key, char *value) {
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        json[key] = value;

        File writeFile = SPIFFS.open(_filename, "w");
        if (!writeFile) {
                if (useSerial) {
                        Serial.println("Failed to open config file for writing");
                }
                return false;
        }

        json.printTo(writeFile);
        return true;
}

bool myJSON::parser(char *json_data, char *key){
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(json_data);

        Serial.println(json[key]);
        
        File writeFile = SPIFFS.open(_filename, "w");
        // if (!writeFile) {
        //         if (useSerial) {
        //                 Serial.println("Failed to open config file for writing");
        //         }
        //         return false;
        // }
        return true;


        // long time = json["time"];
        // Serial.println(time);
}
bool myJSON::exists(char *path){
        return SPIFFS.exists(path);
}

bool myJSON::remove(char *path){
        return SPIFFS.remove(path);
}

bool myJSON::format(){
        return SPIFFS.format();
}
