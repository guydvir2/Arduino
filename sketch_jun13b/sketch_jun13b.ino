#include <Arduino.h>

#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

bool useSer = true;
#define PRNT(a) if(useSer) Serial.print(a)
#define PRNTL(a) Serial.println(a)

bool extract_JSON_from_flash(char *filename, JsonDocument &DOC)
{
  File readFile = LittleFS.open(filename, "r");
  DeserializationError error = deserializeJson(DOC, readFile);
  readFile.close();

  if (error)
  {
    Serial.print(F("Failed to read JSON file: "));
    Serial.println(filename);
    Serial.println(error.c_str());
    Serial.flush();
    delay(100);
    return 0;
  }
  else
  {
    return 1;
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  LittleFS.begin();
  delay(2000);
  StaticJsonDocument<600> doc;
  extract_JSON_from_flash("/myIOT2_topics.json", doc);
  serializeJsonPretty(doc,Serial);
  PRNT("FUYDVIR_THIS IS TEST");
  PRNTL("TEST2");
}

void loop()
{
  // put your main code here, to run repeatedly:

  delay(100);
}
