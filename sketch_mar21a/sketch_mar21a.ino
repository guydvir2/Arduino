#include "myESP32sleep.h"

#define SLEEP_TIME 2
#define FORCE_AWAKE_TIME 15
#define DEV_NAME "ESP32S"

esp32Sleep go2sleep(SLEEP_TIME, FORCE_AWAKE_TIME, DEV_NAME);

void setup()
{
  Serial.begin(9600);
  Serial.println("\n~~~~~~ Boot ~~~~~~");
  go2sleep.use_wifi = true;
  go2sleep.wifi_ssid = "Xiaomi_D6C8";
  go2sleep.wifi_pass = "guyd5161";
  go2sleep.startServices();
}

void loop()
{
  go2sleep.wait_forSleep();
}
