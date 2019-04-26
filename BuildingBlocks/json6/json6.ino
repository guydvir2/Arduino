#include <ArduinoJson.h>
#include "FS.h"
#define filename "file.json"


StaticJsonDocument<1024> doc;

void createJSON(JsonDocument& _doc) {
        _doc["sensor"] = "gps";
        _doc["time"] = 1351824120;
        _doc["name"] = "guyDvir";

        // JsonArray data_2 = _doc.to<JsonArray>();
        // data_2.add(14);
        // data_2.add(2.3333);

        JsonArray data = _doc.createNestedArray("data");
        data.add(48.756080);
        data.add(2.302038);

        // char jsonChar[100];
        // serializeJson(_doc,jsonChar);
        // Serial.println(jsonChar);

        // JsonArray data_3 = _doc.createNestedArray("data_3");
        // data_3.add(48.756080);
        // data_3.add(2.302038);

        // Generate the minified JSON and send it to the Serial port.
        //  serializeJson(_doc, Serial);
        Serial.println("JSON is created");

        // Generate the prettified JSON and send it to the Serial port.
        // serializeJsonPretty(_doc, Serial);

}

void saveJSON2file(JsonDocument& _doc) {
        File writeFile = SPIFFS.open(filename, "w");
        serializeJson(_doc, writeFile);
        Serial.println("JSON file is saved");
}

void readJSON_file(JsonDocument& _doc) {
        File readFile = SPIFFS.open(filename, "r");
        DeserializationError error = deserializeJson(_doc, readFile);
        if (error) {
                Serial.println(F("Failed to read file"));
        }
        serializeJson(_doc, readFile);
        Serial.println("JSON file was read");
}

void printJSON(JsonDocument& _doc) {
        serializeJson(_doc, Serial);
}

void PrettyprintJSON(JsonDocument& _doc) {
        serializeJsonPretty(_doc, Serial);
}


void setup() {
        Serial.begin(9600);
        if (!SPIFFS.begin()) {
                Serial.println("Failed to mount file system");
        }
        float lista[10];//={1.23, 4.65, 5.56};

        // createJSON(doc);
        // saveJSON2file(doc);
        readJSON_file(doc);
        printJSON(doc);
        // PrettyprintJSON(doc);
        // JsonArray boot_time = doc.createNestedArray("bootTime");

        // for (int u=0; u<=doc["bootTime"].size(); u++) {
        //         doc["bootTime"][u]=lista[u];
        // }

        // doc["bootTime"][doc["bootTime"].size()+1]=millis
        int a = random(1, 200000);
        doc["bootTime"].add(millis());
        PrettyprintJSON(doc);


        // Serial.println(lista[0]);
        // Serial.println(doc["data"].size());
        saveJSON2file(doc);

        // printJSON(doc);
        // PrettyprintJSON(doc);

}

void loop() {
        // not used in this example
}
