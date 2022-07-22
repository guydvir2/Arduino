#include <Arduino.h>

// #include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

// bool useSer = true;
// #define PRNT(a) if(useSer) Serial.print(a)
// #define PRNTL(a) Serial.println(a)

// bool extract_JSON_from_flash(char *filename, JsonDocument &DOC)
// {
//   File readFile = LittleFS.open(filename, "r");
//   DeserializationError error = deserializeJson(DOC, readFile);
//   readFile.close();

//   if (error)
//   {
//     Serial.print(F("Failed to read JSON file: "));
//     Serial.println(filename);
//     Serial.println(error.c_str());
//     Serial.flush();
//     delay(100);
//     return 0;
//   }
//   else
//   {
//     return 1;
//   }
// }
struct oper_string
{
  bool state;     /* On or Off */
  uint8_t step;   /* Step, in case of PWM */
  uint8_t reason; /* What triggered the button */
  time_t ontime;  /* Start Clk */
  time_t offtime; /* Off Clk */
} OperString;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  LittleFS.begin();
  delay(200);

  OperString = {0, 0, 0, 0, 0};
  // StaticJsonDocument<600> doc;
  // extract_JSON_from_flash("/myIOT2_topics.json", doc);
  // serializeJsonPretty(doc,Serial);
  // PRNT("FUYDVIR_THIS IS TEST");
  // PRNTL("TEST2");
  OperString = {1, 10, 20, 1231230, 23423423};

  File fileWrite = LittleFS.open("/guy.txt", "w");
  fileWrite.write((byte *)&OperString, sizeof(OperString));
  fileWrite.close();

  OperString = {0, 0, 0, 0, 0};

  File fileread = LittleFS.open("/guy.txt", "r");
  fileread.read((byte *)&OperString, sizeof(OperString));
  fileread.close();

  Serial.println(OperString.state);
  Serial.println(OperString.step);
  Serial.println(OperString.reason);
  Serial.println(OperString.ontime);
  Serial.println(OperString.offtime);
}

void loop()
{
  // put your main code here, to run repeatedly:

  delay(100);
}
