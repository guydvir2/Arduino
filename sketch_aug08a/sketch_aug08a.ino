#include <ArduinoJson.h>
#include "FS.h"

#define DOC_SIZE 400
#define FILENAME "clock.json"

StaticJsonDocument<DOC_SIZE> doc;
char *clock_keys[] = {"on_hours","on_minutes","off_hours","off_minutes","on_flag"};
int clocks [6][2];


void clock2JSON(int on_h, int on_min, int off_h, int off_min, bool flag, StaticJsonDocument<DOC_SIZE> &_doc, const char* name="JSONDOCNAME"){

        JsonArray clock_0 = _doc.createNestedArray(name);
        JsonObject nested = clock_0.createNestedObject();
        nested[clock_keys[0]]=on_h;
        nested[clock_keys[1]]=on_min;
        nested[clock_keys[2]]=off_h;
        nested[clock_keys[3]]=off_min;
        nested[clock_keys[4]]=flag;
}
void saveJSON2file(StaticJsonDocument<DOC_SIZE>& _doc, char *filename=FILENAME) {
        File writeFile = SPIFFS.open(filename, "w");
        serializeJson(_doc, writeFile);
        delay(50);
}
bool readJSON_file(StaticJsonDocument<DOC_SIZE>& _doc, char *filename=FILENAME){
        File readFile = SPIFFS.open(filename, "r");
        DeserializationError error = deserializeJson(_doc, readFile);
        if (error) {
                Serial.println(F("Failed to read file"));
                return 0;
        }
        else{
                serializeJson(_doc, readFile);
                delay(50);
                return 1;
        }
}
void getJSONclock(StaticJsonDocument<DOC_SIZE>& _doc, int array[6][2]){

// extract the values
        JsonArray arr = doc.as<JsonArray>();
        for(JsonVariant v : arr) {
                Serial.println(v.as<int>());
                Serial.println("HI");
        }

        // for (int n=0; n<5; n++){
        //   array[n][0]=_doc["clock_00000"][clock_keys[n]];
        //   Serial.print(clock_keys[n]);
        //   Serial.print(":");
        //   Serial.println(array[n][0]);
        // }

}

void setup() {
        Serial.begin(9600);
        Serial.println();
        if (!SPIFFS.begin()) {
                Serial.println("Failed to mount file system");
        }

        clock2JSON(18,0,21,0,true,doc,"clock_0");
        clock2JSON(22,0,23,0,true,doc,"clock_1");
        saveJSON2file(doc);
        readJSON_file(doc);
        getJSONclock(doc,clocks);

        // serializeJsonPretty(doc,Serial);

}

void loop() {

}
