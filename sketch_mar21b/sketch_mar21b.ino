#include <WiFiManager.h>
#define MQTT_SERVER "192.168.3.200"
WiFiManager wm;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //  WiFiManager wm;
  WiFiManagerParameter custom_mqtt_server("mqtt_server", "mqtt server", MQTT_SERVER, 40);
  WiFiManagerParameter custom_mqtt_user("mqtt_user", "mqtt user", "", 10);
  WiFiManagerParameter custom_mqtt_pwd("mqtt_pwd", "mqtt password", "", 10);
  WiFiManagerParameter custom_mqtt_topic("mqtt_topic", "mqtt topic", "", 10);
  WiFiManagerParameter custom_mqtt_group("mqtt_group", "mqtt group", "", 10);
  WiFiManagerParameter custom_mqtt_root("mqtt_root", "mqtt rootTopic", "", 10);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pwd);
  wm.addParameter(&custom_mqtt_topic);
  wm.addParameter(&custom_mqtt_group);
  wm.addParameter(&custom_mqtt_root);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  //  wm.resetSettings();

  

  //  if (!wm.autoConnect("AutoConnectAP")) {
  //    Serial.println("failed to connect and hit timeout");
  //    delay(3000);
  //    //reset and try again, or maybe put it to deep sleep
  //    ESP.restart();
  //    delay(5000);
  //  }

  bool res;
  res = wm.autoConnect("test12453", "password"); // password protected ap
  wm.setConfigPortalTimeout(120);
  wm.setConfigPortalBlocking(false);
  //  wm.setSaveParamsCallback(saveParamsCallback);

  //  Serial.println("Get Params:");
  //  Serial.print(custom_mqtt_server.getID());
  //  Serial.print(" : ");
  //  Serial.println(custom_mqtt_server.getValue());
}

void loop() {
  // put your main code here, to run repeatedly:
  wm.process();
  Serial.println("hi");
  delay(100);

}
