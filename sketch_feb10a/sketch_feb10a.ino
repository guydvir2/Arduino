
#include <ArduinoJson.h>
#define MCU_NAME "ARUINO_WIFI"

StaticJsonDocument<300> doc;
#define RelOff LOW
#define RelOn !RelOff
const byte upPin = 2;
const byte downPin = 3;

void relay_commands(int cmd)
{
  if (cmd == 0)
  {
    digitalWrite(upPin, RelOff);
    digitalWrite(downPin, RelOff);
  }
  else if (cmd == 1) /*UP*/
  {
    digitalWrite(downPin, RelOff);
    digitalWrite(upPin, RelOn);
  }
  else if (cmd == 2) /*DOWN*/
  {
    digitalWrite(upPin, RelOff);
    digitalWrite(downPin, RelOn);
  }
}
void read_incmoing()
{
  if (Serial.available() > 0)
  {
    int x = Serial.read();
    Serial.println(x);
    // if (x < 3)
    // {
    //   relay_commands(x);
    // }
    // else
    // {
    //   Serial.println(MCU_NAME);
    // }
    // DeserializationError err = deserializeJson(doc, Serial);
    // // serializeJsonPretty(doc, Serial);
    // // Serial.print((char)Serial.read());

    // if (err == DeserializationError::Ok)
    // {
    //   relay_commands(doc["cmd"].as<int>());
    //   // const char *N=doc["name"];
    //   // Serial.print("name = ");
    //   // Serial.println(N);//doc["name"].as<const char*>());
    //   // Serial.print("time = ");
    //   // Serial.println(doc["time"].as<long>());
    // }
    // else
    // {
    //   Serial.print("deserializeJson() returned ");
    //   Serial.println(err.c_str());
    //   // Flush all bytes in the "link" serial port buffer
    //   while (Serial.available() > 0)
    //     Serial.read();
    // }
  }
}
void send_msg(char *msg, int timeout)
{
  static long last = 0;
  static int lastCMD = 0;
  while (millis() - last > 100 * timeout)
  {
    last = millis();
    if (lastCMD < 3)
    {
      lastCMD++;
    }
    else
    {
      lastCMD = 0;
      // }
      // doc["name"] = MCU_NAME;
      // doc["cmd"] = lastCMD;
      // doc["msg"] = msg;
      // serializeJson(doc, Serial);
    }
    Serial.write(lastCMD);
  }
}
void setup()
{
  Serial.begin(115200);

  while (!Serial)
    continue;
    Serial.println(3%10);
    Serial.println(13%10);
    
}

void loop()
{
  // Serial.println(rand());
  send_msg("HI", 10);
  read_incmoing();
}
