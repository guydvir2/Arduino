#include <homeController.h>
#include <myIOT2.h>
#include "myIOT_settings.h"

homeCtl controller;

uint8_t input_pins[] = {0, 2, 13, 15, 3, 1};
uint8_t output_pins[] = {4, 5, 14, 12};
uint8_t RF[4] = {255, 255, 255, 255};

void print_telemetry(Ctl_MSGstr &MSG)
{
  Serial.println("±±±±±±±±±±±± Start ±±±±±±±±±±±±");
  Serial.print("id:\t\t");
  Serial.println(MSG.id);
  Serial.print("type:\t\t");
  Serial.println(MSG.type);
  Serial.print("virtCMD:\t\t");
  Serial.println(MSG.virtCMD);
  Serial.print("state:\t\t");
  Serial.println(MSG.state);
  Serial.print("reason:\t\t");
  Serial.println(MSG.reason);
  Serial.println("±±±±±±±±±±±± END  ±±±±±±±±±±±±");
}
void new_telemetry_handler()
{
  Ctl_MSGstr localMSG;
  controller.get_telemetry(localMSG);
  print_telemetry(localMSG);
  controller.clear_telemetryMSG();
}
char a[]="guy";
char &b=*a;
void setup()
{
  Serial.begin(115200);

  controller.set_RFch(RF, 4);
  controller.set_inputs(input_pins, 6);
  controller.set_outputs(output_pins, 4);
  controller.create_SW("myHome/Light", 1, false, 5, 0);
  controller.create_Win("myHome/Win", false, true);
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
