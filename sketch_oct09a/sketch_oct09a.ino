#include <homeController.h>

homeCtl controller;
uint8_t input_pins[] = {0, 2, 13, 15, 3, 1};
uint8_t output_pins[] = {4, 5, 14, 12};
uint8_t RF[4] = {255, 255, 255, 255};

Ctl_MSGstr localMSG;

void recv_telemetry()
{
  
}

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  controller.set_RFch(RF, 4);
  controller.set_inputs(input_pins, 6);
  controller.set_outputs(output_pins, 4);
  controller.create_SW("myHome/Light", 1, false, 5, 0);
  controller.create_Win("myHome/Win", false, true);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (controller.loop())
  {
    recv_telemetry();
  }
}
