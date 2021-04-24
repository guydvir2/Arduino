#include "winStates.h"
#define VER "Arduino_v0.7"

// ~~~~ Services ~~~~~
#define DUAL_SW false
#define ARD_MICRO false
#define ERR_PROTECT true
// ~~~~~~~~~~~~~~~~~~~

#define RELAY_ON HIGH
#define SW_PRESSED LOW
#define REL_DOWN_PIN 6 /* OUTUPT to relay device */
#define SW_DOWN_PIN 12 /* Switch INPUT to Arduino */
#define REL_UP_PIN 7   /* OUTUPT to relay device */
#define SW_UP_PIN 13   /* Switch INPUT to Arduino */

#if DUAL_SW
#define SW2_UP_PIN 10
#define SW2_DOWN_PIN 11
bool swUp2_lastState = !SW_PRESSED;
bool swDown2_lastState = !SW_PRESSED;
#endif

const byte change_dir_delay = 100;  //ms
const byte debounce_delay = 50;     //ms
const byte loop_delay = LOOP_DELAY; //ms - 10 time faster than ESP8266. DO NOT change

bool swUp_lastState = !SW_PRESSED;
bool swDown_lastState = !SW_PRESSED;

void (*resetFunc)(void) = 0;

// ~~~~~~ Handling Inputs & Outputs ~~~~~~~
void start_gpio()
{
  pinMode(SW_UP_PIN, INPUT_PULLUP);
  pinMode(SW_DOWN_PIN, INPUT_PULLUP);

#if DUAL_SW
  pinMode(SW2_UP_PIN, INPUT_PULLUP);
  pinMode(SW2_DOWN_PIN, INPUT_PULLUP);
#endif

  pinMode(REL_UP_PIN, OUTPUT);
  pinMode(REL_DOWN_PIN, OUTPUT);

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
void makeSwitch(byte state)
{
  send_msg(state);
  state = state % MQTT_OFFSET; // Case it was sent as MQTT msg from ESP8266 10,11,12 -> 1,2,3

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
  }
}
void errorProtection()
{
  if (digitalRead(REL_UP_PIN) == RELAY_ON && digitalRead(REL_DOWN_PIN) == RELAY_ON)
  {
    makeSwitch(WIN_STOP);
  }
  if (digitalRead(SW_UP_PIN) == SW_PRESSED && digitalRead(SW_DOWN_PIN) == SW_PRESSED)
  {
    resetFunc();
  }
#if DUAL_SW
  if (digitalRead(SW2_UP_PIN) == SW_PRESSED && digitalRead(SW2_DOWN_PIN) == SW_PRESSED)
  {
    resetFunc();
  }
#endif
}
void act_inputChange(int inPin, bool &state)
{
  if (state == SW_PRESSED)
  {
    if (inPin == SW_UP_PIN)
    {
      if (getWin_state() != WIN_UP)
      {
        makeSwitch(WIN_UP);
      }
    }
    else if (inPin == SW_DOWN_PIN)
    {
      if (getWin_state() != WIN_DOWN)
      {
        makeSwitch(WIN_DOWN);
      }
    }
  }
  else
  {
    makeSwitch(WIN_STOP);
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
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~  Serial Communication ~~~~~~~~
void Serial_callbacks(byte &x)
{
  if ((x >= WIN_STOP && x <= WIN_DOWN) || (x >= (WIN_STOP + MQTT_OFFSET) && (x <= WIN_DOWN + MQTT_OFFSET)))
  {
    makeSwitch(x);                               /* x={0,1,2,10,11,12}*/
  }
  else if (x == QUERY)
  {
    Serial.write(getWin_state() + QUERY_OFFSET); /* {40,41,42,43} */
  }
  else if (x == RESET)
  {
    send_msg(x);
    delay(10);
    resetFunc();
  }
}
void readSerial()
{
  if (Serial.available())
  {
    byte x = Serial.read();
    Serial_callbacks(x);
  }
}
void send_msg(byte &msg)
{
  Serial.write(msg);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void setup()
{
  start_gpio();
  Serial.begin(9600);
#if ARD_MICRO
  while (!Serial)
    ; /*Relvant for Pro-Micro board */
#endif
  delay(8000); /* Time to ESP8266 to get connected to MQTT&WiFi */
  Serial.write(BOOT);
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
