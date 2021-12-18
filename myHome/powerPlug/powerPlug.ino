#define swPin 0
#define relayPin 12
#define ledPin 13
#define RelayOn HIGH /* Relay contacts . Red Led turns on ( in hardware ) */
#define swON LOW     /* Press Button */
#define ledON LOW    /* green led will be on when Relay is off */
#define vER "powerPlug_V0.2"

/*
 ~~~~~ SONOFF HARDWARE ~~~~~
 #define RELAY1 12
 #define RELAY2 5
 #define INPUT1 0  // 0 for onBoard Button
 #define INPUT2 14 // 14 for extButton
 #define indic_LEDpin 13
 */

bool OnatBoot = false;

#include <myIOT2.h>
#include "powerPlug_param.h"
#include <myTimeoutSwitch.h>
#include "myIOT_settings.h"

timeOUTSwitch TOswitch;

void simplifyClock(char *days, char *clk, char retVal[25])
{
  if (strcmp(days, "0d") != 0)
  {
    sprintf(retVal, "%s %s", days, clk);
  }
  else
  {
    sprintf(retVal, "%s", clk);
  }
}
void startTO(uint8_t src, uint8_t i)
{
  char msg[100];
  char s1[15];
  char s2[7];
  char clk[25];

  char *srcs[] = {"Button", "Timeout", "MQTT"};

  digitalWrite(relayPin, RelayOn);
  digitalWrite(ledPin, !ledON);

  iot.convert_epoch2clock(TOswitch.TO_duration, 0, s1, s2);
  simplifyClock(s2, s1, clk);
  sprintf(msg, "%s: Switched [ON] for [%s]", srcs[src], clk);
  iot.pub_msg(msg);
}
void endTO(uint8_t src, uint8_t i)
{
  char msg[100];
  char s1[15];
  char s2[7];
  char clk[25];
  char clk2[25];

  char *srcs[] = {"Button", "Timeout", "MQTT"};

  digitalWrite(relayPin, !RelayOn);
  digitalWrite(ledPin, ledON);

  int a = iot.now() - TOswitch.onClk();
  iot.convert_epoch2clock(a, 0, s1, s2);
  simplifyClock(s2, s1, clk);

  if (TOswitch.remTime() > 0) /* Forced-End before time */
  {
    iot.convert_epoch2clock(TOswitch.remTime(), 0, s1, s2);
    simplifyClock(s2, s1, clk2);
    sprintf(msg, "%s: Switched [OFF] after [%s], remained [%s]", srcs[src], clk, clk2);
  }
  else /* End by timeout */
  {
    sprintf(msg, "%s: Switched [OFF] ended after [%s]", srcs[src], clk);
  }
  iot.pub_msg(msg);
}
void TOsw_init()
{
  TOswitch.useInput = true;
  TOswitch.def_TO_minutes = 150;
  TOswitch.maxON_minutes = 240;
  TOswitch.trigType = 0;
  TOswitch.def_funcs(startTO, endTO);
  TOswitch.startIO(swPin);
}

void startGPIO()
{
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
}

void setup()
{
  startGPIO();
  // startRead_parameters();
  startIOTservices();
  TOsw_init();
  // endRead_parameters();
}
void loop()
{
  iot.looper();
  TOswitch.looper();
  delay(100);
}
