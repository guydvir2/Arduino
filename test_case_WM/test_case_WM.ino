#include <WiFiManager.h>
#include "secrets_t.h"

#define MQTT_TOPIC "familyRoom"
#define MQTT_GROUP "Windows"
#define MQTT_ROOT "myHome"

WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("mqtt_server", "mqtt server", mserver, 20);
WiFiManagerParameter custom_mqtt_user("mqtt_user", "mqtt user", muser, 20);
WiFiManagerParameter custom_mqtt_pwd("mqtt_pwd", "mqtt password", mpass, 20);
WiFiManagerParameter custom_mqtt_topic("mqtt_topic", "mqtt topic", mtopic, 20);
WiFiManagerParameter custom_mqtt_group("mqtt_group", "mqtt group", mgtopic, 20);
WiFiManagerParameter custom_mqtt_root("mqtt_root", "mqtt rootTopic", mtroot, 20);

bool blocking_mode = false;
bool saved_data = false;

void erase_wifi_data()
{
  wm.resetSettings();
  Serial.println("DATA_DELETED");
}
void press_reset_button(byte gpio)
{
  const bool PRESSED = LOW;

  pinMode(gpio, INPUT_PULLUP);
  if (digitalRead(gpio) == PRESSED)
  {
    delay(3000);
    if (digitalRead(gpio) == PRESSED)
    {
      erase_wifi_data();
    }
  }
}
void saveParamsCallback(char *server, char *user, char *pass, char *topic, char *group, char *root)
{
}
void services()
{
  wm.setWiFiAutoReconnect(true);
  wm.setConfigPortalTimeout(120);
  wm.setConnectTimeout(15);
  wm.setSaveParamsCallback(saveParamsCallback);
}
void WiFi_init(char *mserver, char *muser, char *mpass, char *mtopic, char *mgtopic, char *mtroot, byte iopin)
{
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  services(wm);
  press_reset_button(iopin, wm);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pwd);
  wm.addParameter(&custom_mqtt_topic);
  wm.addParameter(&custom_mqtt_group);
  wm.addParameter(&custom_mqtt_root);

  char APname[50];
  sprintf(APname, "%s/%s/%s_AP", mtroot, mgtopic, mtopic);

  if (wm.getWiFiIsSaved() == false)
  {
    wm.setConfigPortalBlocking(true);
    Serial.println("NO_SAVED_DATA");
    saved_data = false;
  }
  else
  {
    wm.setConfigPortalBlocking(false);
    Serial.println("SAVED_DATA");
    saved_data = true;
  }
  if (wm.autoConnect(APname))
  {
    Serial.println("connected...yeey :)");
    connect_status = true;
    blocking_mode = false;
  }
  else
  {
    Serial.println("fail connected...yeey :)");
    connect_status = false; // entered by me
    blocking_mode = true;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi_init(MQTT_SERVER1, MQTT_USER, MQTT_PASS, MQTT_TOPIC, MQTT_GROUP, MQTT_ROOT, 39);

  //reset settings - wipe credentials for testing
  // _wm.resetSettings();
}
void loop()
{
  // if (blocking_mode)
  // {
  //   wm.process();
  // }
  //  // put your main code here, to run repeatedly:
  //  static bool inside = false;
  //  if ( millis() > 20000 && inside == false && connect_status == true) { // just a timeout trigger
  //    inside = true;
  //    run_full_setup();
  //    Serial.println("ENTERED");
  //  }
  if (WiFi.isConnected())
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("NOT_Connected");
  }
  delay(300);
}
