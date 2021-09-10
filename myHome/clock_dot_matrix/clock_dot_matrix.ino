#include <myIOT2.h>
#include <myTimeoutSwitch.h>
#include <MD_Parola.h>  /* Display */
#include <MD_MAX72xx.h> /* Display */
#include <SPI.h>        /* Display */

#define VER "LEDClock_V0.4"

#define DEV_TOPIC "dotClock"
#define GROUP_TOPIC "intLights"
#define PREFIX_TOPIC "myHome"
#define ADD_MQTT_FUNC addiotnalMQTT

#define LED_PIN D1
#define BUTTON_PIN D2
#define SWITCH_PRESSED LOW
#define OUTPUT_ON HIGH
#define LIGHT_ON_TIMEOUT 30 // minutes

#define CS_PIN D4
#define CLK_PIN D5
#define DATA_PIN D7
#define MAX_DEVICES 4
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

myIOT2 iot;
timeOUTSwitch TOsw;
MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

char dispText[10];
char *srcs[] = {"Button", "MQTT"};

void start_gpio()
{
  pinMode(LED_PIN, OUTPUT);
}

void on_cb(uint8_t src, uint8_t i = 0)
{
  if (digitalRead(LED_PIN) == !OUTPUT_ON)
  {
    digitalWrite(LED_PIN, OUTPUT_ON);
    postMSG("ON", srcs[src]);
    iot.pub_state("on");
  }
}
void off_cb(uint8_t src, uint8_t i = 0)
{
  if (digitalRead(LED_PIN) == OUTPUT_ON)
  {
    digitalWrite(LED_PIN, !OUTPUT_ON);
    postMSG("OFF", srcs[src]);
    iot.pub_state("off");
  }
}
void start_timeOUT()
{
  TOsw.max_pCount = 1; /* Single ON/ OFF (notPWM)*/
  TOsw.trigType = 0;
  TOsw.useInput = true;
  TOsw.maxON_minutes = 180;
  TOsw.def_TO_minutes = LIGHT_ON_TIMEOUT;
  TOsw.startIO(BUTTON_PIN, SWITCH_PRESSED);
  TOsw.def_funcs(on_cb, off_cb);
}
void postMSG(char *state, char *source)
{
  char a[50];
  sprintf(a, "%s: LEDS [%s], TimeOut [%d min]", source, state, LIGHT_ON_TIMEOUT);
  iot.pub_msg(a);
}

void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = false;
  iot.useDebug = false;
  iot.useBootClockLog = true;
  iot.debug_level = 0;
  iot.useNetworkReset = true;
  iot.noNetwork_reset = 60;
  strcpy(iot.deviceTopic, DEV_TOPIC);
  strcpy(iot.prefixTopic, PREFIX_TOPIC);
  strcpy(iot.addGroupTopic, GROUP_TOPIC);
  iot.start_services(ADD_MQTT_FUNC);
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: LED strip is [%s]", digitalRead(LED_PIN) ? "ON" : "OFF");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help2: Commands #3 - [on, off, gpio]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "Ver: Ver:%s", VER);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "gpio") == 0)
  {
    sprintf(msg, "GPIO's: Button[%d], Relay[%d]", BUTTON_PIN, LED_PIN);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "on") == 0)
  {
    on_cb(1, 0);
  }
  else if (strcmp(incoming_msg, "off") == 0)
  {
    off_cb(1, 0);
  }
}

void start_dotMatrix()
{
  dotMatrix.begin();
  dotMatrix.setIntensity(2);
}
void update_clk_newMinute(time_t &t)
{
  struct tm *timeinfo;
  timeinfo = localtime(&t);

  sprintf(dispText, "%02d:%02d\0", timeinfo->tm_hour, timeinfo->tm_min);
  dotMatrix.displayText(dispText, PA_CENTER, 30, 0, PA_SCROLL_LEFT);
  while (!dotMatrix.displayAnimate())
  {
  }
}
void display_date(time_t &t)
{
  sprintf(dispText, "%02d/%02d\0", day(t), month(t));
  dotMatrix.displayText(dispText, PA_CENTER, 30, 0, PA_SCROLL_RIGHT);
  dotMatrix.displayText(dispText, PA_CENTER, 20, 0, PA_OPENING);
  while (!dotMatrix.displayAnimate())
  {
  }
}
void update_clk_blink(time_t &t)
{
  static bool blink = false;
  struct tm *timeinfo;
  timeinfo = localtime(&t);

  sprintf(dispText, "%02d%c%02d\0", timeinfo->tm_hour, blink ? ':' : ' ', timeinfo->tm_min);
  dotMatrix.displayText(dispText, PA_CENTER, 0, 0, PA_PRINT);
  dotMatrix.displayAnimate();
  blink = !blink;
}
void updateDisplay()
{
  const int blink_delay = 500;
  static uint8_t lastMin = 0;
  static bool dateshown = false;
  static unsigned long blink_clock = 0;

  time_t t;
  struct tm *timeinfo;

  time(&t);
  timeinfo = localtime(&t);

  if (lastMin != timeinfo->tm_min) /* update clk every minute with animation */
  {
    update_clk_newMinute(t);
    lastMin = timeinfo->tm_min;
    dateshown = false;
  }
  else if (timeinfo->tm_sec > 5 && timeinfo->tm_sec <= 10) /* display date at first 5sec every minute */
  {
    if (dateshown == false)
    {
      display_date(t);
      dateshown = true;
    }
  }
  else if (millis() >= blink_clock + blink_delay) /* update blinks */
  {
    update_clk_blink(t);
    blink_clock = millis();
  }
}

void setup()
{
  startIOTservices();
  start_dotMatrix();
  start_gpio();
  start_timeOUT();
}
void loop()
{
  iot.looper();
  TOsw.looper();
  updateDisplay();
}
