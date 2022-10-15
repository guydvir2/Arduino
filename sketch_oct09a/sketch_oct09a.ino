#include <homeController.h>
#include <myIOT2.h>
#include "manual_mode.h"
#include "parameterRead.h"
#include "myIOT_settings.h"

homeCtl controller;

void print_telemetry(Cotroller_Ent_telemetry &MSG)
{
  Serial.println("±±±±±±±±±±±± Start ±±±±±±±±±±±±");
  Serial.print("id:\t\t");
  Serial.println(MSG.id);
  Serial.print("type:\t\t");
  Serial.println(MSG.type);
  Serial.print("state:\t\t");
  Serial.println(MSG.state);
  Serial.print("trig:\t\t");
  Serial.println(MSG.trig);
  Serial.print("timeout:\t");
  Serial.println(MSG.timeout);
  Serial.println("±±±±±±±±±±±± END  ±±±±±±±±±±±±");
}
void new_telemetry_handler()
{
  Cotroller_Ent_telemetry localMSG;
  controller.get_telemetry(localMSG);

  print_telemetry(localMSG);
  post_telemetry_2MQTT(localMSG);
  controller.clear_telemetryMSG();
}

void setup()
{
  init_Serial_DBG_MODE();
  read_all_parameters();
  startIOTservices();
}
void loop()
{
  if (controller.loop())
  {
    new_telemetry_handler();
  }
  iot.looper();
}
