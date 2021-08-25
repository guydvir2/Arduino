// ±±±±±± button configuration ±±±±±±±
#include <buttonPresses.h>

#define LED_PIN D1
#define BUTTON_PIN D2
#define SWITCH_ON HIGH
#define LIGHT_ON_TIMEOUT 30 // minutes

buttonPresses ButPress(BUTTON_PIN, 0);
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

// ±±±±±± iot configuration ±±±±±±±
#include <myIOT2.h>
#include <TimeLib.h>

#define DEV_TOPIC "dotClock"
#define GROUP_TOPIC "intLights"
#define PREFIX_TOPIC "myHome"
#define ADD_MQTT_FUNC addiotnalMQTT

myIOT2 iot;
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

// ±±±±±± Display configuration ±±±±±±±
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define CS_PIN D4
#define CLK_PIN D5
#define DATA_PIN D7
#define MAX_DEVICES 4
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

#define VER "LEDClockV0.3"

char dispText[10];
unsigned long ONclock = 0;

void start_gpio()
{
  ButPress.start();
  pinMode(LED_PIN, OUTPUT);
}
void turnLEDsON(char *txt = "Button")
{
  if (digitalRead(LED_PIN) == !SWITCH_ON)
  {
    postMSG("ON", txt);
    digitalWrite(LED_PIN, SWITCH_ON);
    ONclock = millis();
    iot.pub_state("on");
  }
}
void turnLEDsOFF(char *txt = "TimeOUT")
{
  if (digitalRead(LED_PIN) == SWITCH_ON)
  {
    postMSG("Off", txt);
    digitalWrite(LED_PIN, !SWITCH_ON);
    ONclock = 0;
    iot.pub_state("off");
  }
}
void checkButton()
{
  if (ButPress.getValue() == 1)
  {
    if (digitalRead(LED_PIN) == SWITCH_ON)
    {
      turnLEDsOFF("Button");
    }
    else
    {
      turnLEDsON("Button");
    }
  }
}
void checkTimeOUT()
{
  if (ONclock > 0 && millis() > ONclock + LIGHT_ON_TIMEOUT * 1000 * 60L)
  {
    turnLEDsOFF();
  }
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
  iot.noNetwork_reset = 10;
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
    turnLEDsON("MQTT");
  }
  else if (strcmp(incoming_msg, "off") == 0)
  {
    turnLEDsOFF("MQTT");
  }
}
void postMSG(char *state, char *source)
{
  char a[50];
  sprintf(a, "%s: LEDS [%s], TimeOut [%d min]", source, state, LIGHT_ON_TIMEOUT);
  iot.pub_msg(a);
}

void start_dotMatrix()
{
  dotMatrix.begin();
  dotMatrix.setIntensity(2);
}
void update_clk_newMinute(time_t &t)
{
  sprintf(dispText, "%02d:%02d\0", hour(t), minute(t));
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

  sprintf(dispText, "%02d%c%02d\0", hour(t), blink ? ':' : ' ', minute(t));
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

  time_t t = iot.now();

  if (lastMin != minute(t))                         /* update clk every minute with animation */
  {
    update_clk_newMinute(t);
    lastMin = minute(t);
    dateshown = false;
  }
  else if (second(t) > 5 && second(t) <= 10)        /* display date at first 5sec every minute */
  {
    if (dateshown == false)
    {
      display_date(t);
      dateshown = true;
    }
  }
  else if (millis() >= blink_clock + blink_delay)   /* update blinks */
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
}
void loop()
{
  iot.looper();
  updateDisplay();
  checkButton();
  checkTimeOUT();
  delay(100);
}
