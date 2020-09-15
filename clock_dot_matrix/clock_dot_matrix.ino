#include <Arduino.h>
#include <myIOT.h>
#include <myPIR.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// ********** Sketch Services  ***********
#define VER "NodeMCU V0.1"
#define ADD_MQTT_FUNC addiotnalMQTT

#define BUTTON_PIN D2
#define SWITCH_PIN D1
#define SWITCH_ON HIGH
#define BUT_PRESSED LOW

#define DEV_TOPIC "dotClock"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN D5
#define DATA_PIN D7
#define CS_PIN D4

const int minutesON = 5;
bool lastDet_state = false;
unsigned long ONclock = 0;
char dispText[10];

myIOT iot;
MD_Parola dotMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void start_gpio()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, OUTPUT);
}
void turnLEDsON(char *txt = "Button")
{
  if (digitalRead(SWITCH_PIN) == LOW)
  {
    char a[20];
    sprintf(a, "%s: LEDS turn [ON], TimeOut [%d min]", txt, minutesON);
    digitalWrite(SWITCH_PIN, SWITCH_ON);
    lastDet_state = 1;
    ONclock = millis();
    iot.pub_msg(a);
    iot.pub_state("on");
  }
}
void turnLEDsOFF(char *txt = "TimeOUT")
{
  if (digitalRead(SWITCH_PIN) == HIGH)
  {
    char a[20];
    sprintf(a, "%s: LEDS turn [OFF]", txt);
    digitalWrite(SWITCH_PIN, !SWITCH_ON);
    lastDet_state = 0;
    ONclock = 0;
    iot.pub_msg(a);
    iot.pub_state("off");
  }
}

void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = false;
  iot.resetFailNTP = true;
  iot.useDebug = false;
  iot.debug_level = 0;
  iot.useNetworkReset = true;
  iot.noNetwork_reset = 1;
  strcpy(iot.deviceTopic, DEV_TOPIC);
  strcpy(iot.prefixTopic, PREFIX_TOPIC);
  strcpy(iot.addGroupTopic, GROUP_TOPIC);
  iot.start_services(ADD_MQTT_FUNC);
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: LED strip is [%s]", digitalRead(SWITCH_PIN) ? "ON" : "OFF");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help: Commands #3 - [on, off, gpio]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "Ver: Ver:%s", VER);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "gpio") == 0)
  {
    sprintf(msg, "GPIO's: Button[%d], Relay[%d]", BUTTON_PIN, SWITCH_PIN);
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "on") == 0)
  {
    turnLEDsON();
    sprintf(msg, "MQTT: LEDstrip switched [ON]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "off") == 0)
  {
    // if (PIR.timeLeft > 0)
    // {
    //   PIR.turnOff();
    // }
    // else
    // {
    //   turnLEDsOFF("MQTT");
    // }
  }
}

void start_dotMatrix()
{
  dotMatrix.begin();
  dotMatrix.setIntensity(2);
}
void updateDisplay()
{
  char clk[10];
  char dat[10];
  const int blink_delay = 500;
  static int lastMin = 0;
  static bool blink = false;
  static bool dateshown = false;
  static unsigned long blink_clock = 0;
  time_t t = now();

  if (lastMin != minute(t))
  {
    sprintf(dispText, "%02d:%02d\0", hour(t), minute(t));
    dotMatrix.displayText(dispText, PA_CENTER, 30, 0, PA_SCROLL_LEFT);
    lastMin = minute(t);
    dateshown = false;
    while (!dotMatrix.displayAnimate())
    {
    }
  }
  else if (second(t) > 5 && second(t) <= 10)
  {
    if (dateshown == false)
    {
      sprintf(dispText, "%02d/%02d\0", day(t), month(t));
      dotMatrix.displayText(dispText, PA_CENTER, 100, 0, PA_OPENING);
      dateshown = true;
      while (!dotMatrix.displayAnimate())
      {
      }
    }
  }
  else if (millis() >= blink_clock + blink_delay)
  {
    sprintf(dispText, "%02d%c%02d\0", hour(t), blink ? ':' : ' ', minute(t));
    dotMatrix.displayText(dispText, PA_CENTER, 0, 0, PA_PRINT);
    dotMatrix.displayAnimate();
    blink = !blink;
    blink_clock = millis();
  }
}

void checkButton()
{
  if (digitalRead(BUTTON_PIN) == BUT_PRESSED)
  {
    delay(50);
    if (digitalRead(BUTTON_PIN) == BUT_PRESSED)
    {
      if (lastDet_state == 1)
      {
        turnLEDsOFF("Button");
      }
      else
      {
        turnLEDsON("Button");
      }
      delay(400);
    }
  }
}
void checkTimeOUT()
{
  if (ONclock > 0 && millis() > ONclock + minutesON * 1000 * 60L)
  {
    turnLEDsOFF();
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
