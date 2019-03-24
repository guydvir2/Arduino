
#include <ArduinoJson.h>
#include "FS.h"


#define FILE_NAME "/test.json"
//const *char key_1= "Name";
//const *char value_1="GuyDvir";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

   if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  readFile(FILE_NAME,"name");
//saveFile(FILE_NAME,"name","guyDvir");


}

bool readFile(char *filename, char *key) {
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
  return true;
}

bool saveFile(char *filename, char key[10], char value[10]) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json[key] = value;

  File writeFile = SPIFFS.open(filename, "w");
  if (!writeFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(writeFile);
  return true;
}

void loop() {
  // put your main code here, to run repeatedly:

}
