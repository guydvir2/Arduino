#include <myIOT.h>
#include "test_param.h"
#include <Arduino.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_0.1"
#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot;
myTelegram teleNotify("812406965:AAEaV-ONCIru8ePuisuMfm0ECygsm5adZHs", "596123373", 1);

void startIOTservices()
{
  startRead_parameters();

  iot.useSerial = paramJSON["useSerial"];
  iot.useWDT = paramJSON["useWDT"];
  iot.useOTA = paramJSON["useOTA"];
  iot.useResetKeeper = paramJSON["useResetKeeper"];
  iot.resetFailNTP = paramJSON["useFailNTP"];
  iot.useDebug = paramJSON["useDebugLog"];
  iot.debug_level = paramJSON["debug_level"];
  iot.useNetworkReset = paramJSON["useNetworkReset"];
  iot.noNetwork_reset = paramJSON["noNetwork_reset"];
  strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
  strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
  strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
  iot.start_services(ADD_MQTT_FUNC);

  endRead_parameters();
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: OK");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help: Commands #3 - [NEW]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "Ver: Ver:%s", VER);
    iot.pub_msg(msg);
  }
}

void telecmds(String in_msg, String from, String chat_id, char *snd_msg)
{
}
void setup()
{
  startIOTservices();
  // Serial.begin(9600);
  // if (WiFi.status() != WL_CONNECTED)
  // {
  //   WiFi.mode(WIFI_STA);
  //   // WiFi.disconnect();
  //   delay(100);

  //   Serial.print("Connecting Wifi: ");
  //   Serial.println(SSID_ID);
  //   WiFi.begin(SSID_ID, PASS_WIFI);

  //   while (WiFi.status() != WL_CONNECTED)
  //   {
  //     Serial.print(".");
  //     delay(500);
  //   }
  //   WiFi.setAutoReconnect(true);
  // }
  teleNotify.begin(telecmds);
  teleNotify.send_msg("BAAAA");
}
void loop()
{
  iot.looper();
  teleNotify.looper();
  delay(100);
}
