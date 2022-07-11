#include <ArduinoJson.h>

const char *onflash_Paramter[] = {nullptr, nullptr};
char thisArray[3][50];

void readJSON_fromfile()
{
  /* This part emulates reading JSON from file */
  StaticJsonDocument<200> JSON_FILE;

  JSON_FILE["Par_1"] = "Value1";
  JSON_FILE["Par_2"] = "Value2";

  sprintf(thisArray[0], JSON_FILE["Par_1"]);
  sprintf(thisArray[1], JSON_FILE["Par_2"]);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStart");
  readJSON_fromfile();

  onflash_Paramter[0] = thisArray[0];
  onflash_Paramter[1] = thisArray[1];

  Serial.println(onflash_Paramter[0]);
  Serial.println(onflash_Paramter[1]);
}

void loop()
{
}
