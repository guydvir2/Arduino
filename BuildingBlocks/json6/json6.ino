#include <ArduinoJson.h>
#include "FS.h"
#define filename "file.json"


StaticJsonDocument<200> doc;

void createJSON(JsonDocument& _doc) {
        _doc["sensor"] = "gps";
        _doc["time"] = 1351824120;
        _doc["name"] = "guyDvir";

        JsonArray data = _doc.createNestedArray("data");
        data.add(48.756080);
        data.add(2.302038);

        // Generate the minified JSON and send it to the Serial port.
        //  serializeJson(_doc, Serial);
        Serial.println("JSON is created");

        // Generate the prettified JSON and send it to the Serial port.
        //  serializeJsonPretty(_doc, Serial);

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

        createJSON(doc);
        saveJSON2file(doc);
        readJSON_file(doc);
        printJSON(doc);
        PrettyprintJSON(doc);

}

void loop() {
        // not used in this example
}
