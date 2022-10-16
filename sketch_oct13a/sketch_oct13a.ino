#include <Arduino.h>
#include <smartSwitch.h>

smartSwitch sw;
smartSwitch *sw_v[2] = {&sw, nullptr};

class a
{
  public:
  a()
  {
  }
  smartSwitch SW;
  smartSwitch *sw_v[2]={&SW,nullptr};

  void init(uint8_t pin, uint8_t type, uint8_t output, uint8_t timeout,uint8_t i=0)
  {
    sw_v[i]->set_name("name");
    sw_v[i]->set_input(pin, type);
    sw_v[i]->set_timeout(timeout);
    sw_v[i]->set_output(output);
    // sw_v[i]->get_prefences();
  }
  void pref(uint8_t i=0)
  {
    sw_v[i]->get_prefences();
  }
  void turn(uint8_t i=0){
    sw_v[i]->turnON_cb(1);
  }
  void loop(uint8_t i=0){
    sw_v[i]->loop();
  }
};

a A;
void setup()
{
  Serial.begin(115200);
  Serial.println("\n\nStart");
  A.init(19,2,17,10);
  A.pref();

 

  // sw_v[0]->turnON_cb(1);
  delay(1000);
  A.turn();
}
void loop()
{
  // sw_v[0]->loop();
  // Serial.println(sw_v[0]->telemtryMSG.state);
  A.loop();
  Serial.println(A.sw_v[0]->telemtryMSG.state);
  delay(500);
}
