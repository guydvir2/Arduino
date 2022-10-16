#include <homeController.h>
// #include <myIOT2.h>
// #include "manual_mode.h"
// #include "parameterRead.h"
// #include "myIOT_settings.h"

homeCtl controller;

uint8_t onFlash_hardware_preset = 2;
void print_telemetry(Cotroller_Ent_telemetry &MSG)
{
  Serial.println("\n±±±±±±± Telemetry Start ±±±±±±±");
  Serial.print("Entity type, id:");
  Serial.print(controller.EntTypes[MSG.type]);
  Serial.print(", ");
  Serial.println(MSG.id);
  Serial.print("state:\t\t");
  Serial.println(MSG.type == WIN_ENT ? controller.winMQTTcmds[MSG.state] : controller.SW_MQTT_cmds[MSG.state]);
  Serial.print("trig:\t\t");
  Serial.println(MSG.type == WIN_ENT ? controller.WinTrigs[MSG.trig] : controller.SW_Types[MSG.trig]);
  Serial.print("timeout:\t");
  Serial.println(MSG.timeout);
  Serial.println("±±±±±±±±±±±± END  ±±±±±±±±±±±±\n");
}
void new_telemetry_handler()
{
  Cotroller_Ent_telemetry localMSG;
  controller.get_telemetry(localMSG);
  print_telemetry(localMSG);
  // post_telemetry_2MQTT(localMSG);
  controller.clear_telemetryMSG();
}

void setup()
{
  // init_Serial_DBG_MODE();
  Serial.begin(115200);
  // read_all_parameters();
  // startIOTservices();
  uint8_t inpins[] = {17, 3};
  uint8_t outpins[] = {32, 33};
  controller.create_SW(inpins, outpins, "", 2);
  controller.SW_switchCB(0, 1, 2);
  // controller.SW_v[0]->turnON_cb(1, 3);
}
void loop()
{
  static bool a = false;
  if (controller.loop())
  {
    new_telemetry_handler();
  }
  // iot.looper();

  if (millis() > 6000 && !a)
  {
    a = true;
    controller.SW_v[0]->turnON_cb(0, 3);
  }
}
