#include <ArduinoJson.h>
#include "winStates.h"

// ~~~~ Services ~~~~~
#define DUAL_SW true
#define ERR_PROTECT true
#define VER "Arduino_v1.0"
#define MCU_TYPE "ProMini_3v3"

#define REL_DOWN_PIN 6  /* OUTUPT to relay device */
#define REL_UP_PIN 7    /* OUTUPT to relay device */
#define SW2_UP_PIN 10   /* Switch2 INPUT to Arduino */
#define SW2_DOWN_PIN 11 /* Switch2 INPUT to Arduino */
#define SW_DOWN_PIN 12  /* Switch1 INPUT to Arduino */
#define SW_UP_PIN 13    /* Switch1 INPUT to Arduino */
#define RELAY_ON HIGH
#define SW_PRESSED LOW

const byte change_dir_delay = 100;  //ms
const byte debounce_delay = 50;     //ms
const byte loop_delay = LOOP_DELAY; //ms - 10 time faster than ESP8266. DO NOT change

bool swUp_lastState = !SW_PRESSED;
bool swDown_lastState = !SW_PRESSED;
#if DUAL_SW
bool swUp2_lastState = !SW_PRESSED;
bool swDown2_lastState = !SW_PRESSED;
#endif

enum sys_states :const byte
{
  WIN_STOP,
  WIN_UP,
  WIN_DOWN,
  WIN_ERR,
};

// ~~~~~~~~~  Serial Communication ~~~~~~~~
void (*resetFunc)(void) = 0;
void uptime(char *ret_clk)
{
  byte days = 0;
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;

  int sec2minutes = 60;
  int sec2hours = (sec2minutes * 60);
  int sec2days = (sec2hours * 24);
  int sec2years = (sec2days * 365);

  unsigned long time_delta = (int)(millis() / 1000);

  days = (int)(time_delta / sec2days);
  hours = (int)((time_delta - days * sec2days) / sec2hours);
  minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
  seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

  sprintf(ret_clk, "%01dd %02d:%02d:%02d", days, hours, minutes, seconds);
}
void sendMSG(char *msg, char *addinfo = NULL)
{
  StaticJsonDocument<JSON_SERIAL_SIZE> doc;
  static int counter = 0;

  doc["from"] = NAME_0;
  doc["msg_num"] = counter++;
  doc["act"] = msg;
  if (addinfo == NULL)
  {
    doc["info"] = "none";
  }
  else
  {
    doc["info"] = addinfo;
  }

  serializeJson(doc, Serial);
}
void Serial_CB(JsonDocument &_doc)
{
  const char *FROM = _doc["from"];
  const char *ACT = _doc["act"];
  const char *INFO = _doc["info"];
  int msg_num = _doc["msg_num"];

  if (strcmp(ACT, "up") == 0)
  {
    if (makeSwitch(WIN_UP))
    {
      sendMSG(ACT, INFO);
    }
  }
  else if (strcmp(ACT, "down") == 0)
  {
    if (makeSwitch(WIN_DOWN))
    {
      sendMSG(ACT, INFO);
    }
  }
  else if (strcmp(ACT, "off") == 0)
  {
    if (makeSwitch(WIN_STOP))
    {
      sendMSG(ACT, INFO);
    }
  }
  else if (strcmp(ACT, "reset_MCU") == 0)
  {
    resetFunc();
  }
  else if (strcmp(ACT, "status") == 0)
  {
    byte a = getWin_state();
    switch (a)
    {
    case WIN_DOWN:
      sendMSG("status", "down");
      break;
    case WIN_UP:
      sendMSG("status", "up");
      break;
    case WIN_STOP:
      sendMSG("status", "off");
      break;

    default:
      sendMSG("status", "error-state");
      break;
    }
  }
  else if (strcmp(ACT, "query") == 0)
  {
    char t[200];
    char clk[25];
    uptime(clk);
    sprintf(t, "ver[%s], MCU[%s], upTime[%s], DualSW[%s], ErrProtect[%s]", VER, MCU_TYPE, clk, DUAL_SW ? "YES" : "NO", ERR_PROTECT ? "YES" : "NO");
    sendMSG("query", t);
  }
}
void readSerial()
{
  if (Serial.available() > 0)
  {
    StaticJsonDocument<JSON_SERIAL_SIZE> doc;
    DeserializationError error = deserializeJson(doc, Serial);
    if (!error)
    {
      Serial_CB(doc);
    }
  }
}

// ~~~~~~ Handling Inputs & Outputs ~~~~~~~
void start_gpio()
{
  pinMode(REL_UP_PIN, OUTPUT);
  pinMode(REL_DOWN_PIN, OUTPUT);
  pinMode(SW_UP_PIN, INPUT_PULLUP);
  pinMode(SW_DOWN_PIN, INPUT_PULLUP);
  swUp_lastState = digitalRead(SW_UP_PIN);
  swDown_lastState = digitalRead(SW_DOWN_PIN);

#if DUAL_SW
  pinMode(SW2_UP_PIN, INPUT_PULLUP);
  pinMode(SW2_DOWN_PIN, INPUT_PULLUP);
  swUp2_lastState = digitalRead(SW2_UP_PIN);
  swDown2_lastState = digitalRead(SW2_DOWN_PIN);
#endif

  allOff();
}
void allOff()
{
  digitalWrite(REL_UP_PIN, !RELAY_ON);
  digitalWrite(REL_DOWN_PIN, !RELAY_ON);
  delay(change_dir_delay);
}
byte getWin_state()
{
  bool relup = digitalRead(REL_UP_PIN);
  bool reldown = digitalRead(REL_DOWN_PIN);

  if (relup == !RELAY_ON && reldown == !RELAY_ON)
  {
    return WIN_STOP;
  }
  else if (relup == RELAY_ON && reldown == !RELAY_ON)
  {
    return WIN_UP;
  }
  else if (relup == !RELAY_ON && reldown == RELAY_ON)
  {
    return WIN_DOWN;
  }
  else
  {
    return WIN_ERR;
  }
}
bool makeSwitch(byte state)
{
  if (getWin_state() != state) /* Not already in that state */
  {
    switch (state)
    {
    case WIN_STOP:
      allOff();
      break;
    case WIN_UP:
      allOff();
      digitalWrite(REL_UP_PIN, RELAY_ON);
      break;
    case WIN_DOWN:
      allOff();
      digitalWrite(REL_DOWN_PIN, RELAY_ON);
      break;
    default:
      allOff();
      break;
    }
    return 1;
  }
  else
  {
    return 0;
  }
}
void errorProtection()
{
  if (digitalRead(REL_UP_PIN) == RELAY_ON && digitalRead(REL_DOWN_PIN) == RELAY_ON)
  {
    makeSwitch(WIN_STOP);
    sendMSG("Error", "Relays");
  }
  if (digitalRead(SW_UP_PIN) == SW_PRESSED && digitalRead(SW_DOWN_PIN) == SW_PRESSED)
  {
    sendMSG("Error", "Buttons");
    delay(100);
    resetFunc();
  }
#if DUAL_SW
  if (digitalRead(SW2_UP_PIN) == SW_PRESSED && digitalRead(SW2_DOWN_PIN) == SW_PRESSED)
  {
    sendMSG("Error", "ExButtons");
    delay(100);
    resetFunc();
  }
#endif
}
void act_inputChange(int inPin, bool &state)
{
  if (state == SW_PRESSED)
  {
    if (inPin == SW_UP_PIN || inPin == SW2_UP_PIN)
    {
      if (getWin_state() != WIN_UP)
      {
        makeSwitch(WIN_UP);
        if (inPin == SW_UP_PIN)
        {
          sendMSG("up", "Button");
        }
        else
        {
          sendMSG("up", "ExButton");
        }
      }
    }
    else if (inPin == SW_DOWN_PIN || inPin == SW2_DOWN_PIN)
    {
      if (getWin_state() != WIN_DOWN)
      {
        makeSwitch(WIN_DOWN);
        if (inPin == SW_UP_PIN)
        {
          sendMSG("down", "Button");
        }
        else
        {
          sendMSG("down", "ExButton");
        }
      }
    }
  }
  else
  {
    makeSwitch(WIN_STOP);
    if (inPin == SW_UP_PIN)
    {
      sendMSG("off", "Button");
    }
    else
    {
      sendMSG("off", "ExButton");
    }
  }
}
void readInput(int inPin, bool &lastState)
{
  bool state = digitalRead(inPin);

  if (state != lastState)
  {
    delay(debounce_delay);
    if (digitalRead(inPin) == state)
    {
      act_inputChange(inPin, state);
      lastState = state;
    }
  }
}

void setup()
{
  start_gpio();
  Serial.begin(9600);
  while (!Serial)
    ;            /*Relvant for Pro-Micro board */
  delay(8000);   /* Time to ESP8266 to get connected to MQTT&WiFi */
  sendMSG("Boot");
}
void loop()
{
  readInput(SW_UP_PIN, swUp_lastState);     /* Read wall UP Switch */
  readInput(SW_DOWN_PIN, swDown_lastState); /* Read wall DOWN Switch */
#if DUAL_SW
  readInput(SW2_UP_PIN, swUp2_lastState);     /* Read wall UP Switch */
  readInput(SW2_DOWN_PIN, swDown2_lastState); /* Read wall DOWN Switch */
#endif
#if ERR_PROTECT
  errorProtection(); /* Avoid Simulatnious UP&DOWN */
#endif
  readSerial();
  delay(loop_delay);
}
