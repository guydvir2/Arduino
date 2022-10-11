#include <homeController.h>
#include <myIOT2.h>
#include "myIOT_settings.h"

#define RETAINED_MSG false

homeCtl controller;

uint8_t RF[4] = {3135496, 3135492, 3135490, 3135489};
uint8_t output_pins[] = {4, 5, 14, 12};
uint8_t input_pins[] = {0, 2, 13, 15, 3, 1};

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
  Serial.println("±±±±±±±±±±±± END  ±±±±±±±±±±±±");
}
void new_telemetry_handler()
{
  Cotroller_Ent_telemetry localMSG;
  controller.get_telemetry(localMSG);
  // print_telemetry(localMSG);
  post_telemetry_2MQTT(localMSG);
  controller.clear_telemetryMSG();
}

void start_controller()
{
  controller.set_RFch(RF, 4);
  controller.set_inputs(input_pins, 6);
  controller.set_outputs(output_pins, 4);
  controller.create_SW("myHome/Light", 1, false, 5, 0);
  controller.create_Win("myHome/Win", false, true);
}

void setup()
{
  Serial.begin(115200);

  start_controller();
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
