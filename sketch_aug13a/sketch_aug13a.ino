// #include <myJSON.h>

#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// StaticJsonDocument<512> tempJDOC;
DynamicJsonDocument tempJDOC(512);

// // // myJSON jsonfile("/json.txt",true);
const char *_filename = "/myFile.json";
bool getValue(const char *key, char value[])
{
  readJSON_file(tempJDOC);
  bool hasKey = tempJDOC.containsKey(key);
  if (hasKey)
  {
    const char *val = tempJDOC[key];
    sprintf(value, "%s", val);
    strcpy(value, val);
    return 1;
  }
  else
  {
    return 0; // when key is not present
  }
}

bool getValue(const char *key, int &value)
{
  readJSON_file(tempJDOC);
  bool hasKey = tempJDOC.containsKey(key);
  if (hasKey)
  {
    value = tempJDOC[key];
    return 1;
  }
  else
  {
    return 0; // when key is not present
  }
}

void saveJSON2file(JsonDocument &_doc)
{
  File writeFile = LittleFS.open(_filename, "w");
  serializeJson(_doc, writeFile);
  writeFile.close();
  delay(50);
  // Serial.println("JSON file saved OK");
  // serializeJsonPretty(_doc, Serial);
}
bool readJSON_file(JsonDocument &_doc)
{
  File readFile = LittleFS.open(_filename, "r");
  DeserializationError error = deserializeJson(_doc, readFile);
  if (error)
  {
    Serial.println(F("Failed to read JSON file"));
    Serial.println(_filename);
    Serial.println(error.c_str());
    readFile.close();
    return 0;
  }
  else
  {
    // serializeJson(_doc, Serial);
    readFile.close();
    return 1;
  }
  delay(50);
}

void setValue(const char *key, const char *value)
{
  tempJDOC[key] = value;
  saveJSON2file(tempJDOC);
}
void setValue(const char *key, int value)
{
  tempJDOC[key] = value;
  saveJSON2file(tempJDOC);
}

void a(JsonDocument &_doc, const char *value)
{
  File file = LittleFS.open(_filename, "w");
  _doc["key"] = value;
  serializeJson(_doc, file);
  file.close();
}
void b(JsonDocument &_doc, char retValue[])
{
  File openFile = LittleFS.open(_filename, "r");
  DeserializationError error = deserializeJson(_doc, openFile);
  if (error)
  {
    Serial.println(error.c_str());
  }
  else
  {
    const char *T = _doc["key"];
    strcpy(retValue, T);
    serializeJsonPretty(_doc, Serial);
  }
  openFile.close();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\n\nStart");
  // LittleFS.format();
  while (!LittleFS.begin())
  {
    Serial.println("ERR");
    delay(50);
  }
  int x = 0;
  char aa[70];

  setValue("key", "12hjgfjhgfjhgfjhgf34");
  getValue("key",aa);
  Serial.println(aa);
  // serializeJsonPretty(tempJDOC,Serial);
  // readJSON_file(tempJDOC);
  // getValue(""key", x);
  // Serial.print("GOT: ");
  // Serial.println(x);"

  // const char *c;
  // a(tempJDOC, "AVBDFBVDF");
  // b(tempJDOC,aa);

  // Serial.println(c);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(100);
}
