#define REL_DOWN 2 /* OUTUPT to relay device */
#define REL_UP 3   /* OUTUPT to relay device */
#define SW_UP 4    /* Switch INPUT to ProMini */
#define SW_DOWN 5  /* Switch INPUT to ProMini */

#define SW_PRESSED LOW
#define RELAY_ON HIGH

#define VER "ProMini_v0.3"

const byte WIN_STOP = 0;
const byte WIN_UP = 1;
const byte WIN_DOWN = 2;
const byte WIN_ERR = 3;

const byte change_dir_delay = 20; //ms
const byte debounce_delay = 20;   //ms
const byte loop_delay = 10;       //ms

bool swUp_lastState = false;
bool swDown_lastState = false;

void start_gpio()
{
  pinMode(SW_UP, INPUT_PULLUP);
  pinMode(SW_DOWN, INPUT_PULLUP);

  pinMode(REL_UP, OUTPUT);
  pinMode(REL_DOWN, OUTPUT);

  allOff();
}
byte check_current_relState()
{
  bool relup = digitalRead(REL_UP);
  bool reldown = digitalRead(REL_DOWN);

  if (relup && reldown == !RELAY_ON)
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
void readInput(int inPin, int outPin, bool &lastState)
{
  bool state = digitalRead(inPin);
  if (state != lastState)
  {
    delay(debounce_delay);
    if (digitalRead(inPin) == state)
    {
      if (state == SW_PRESSED)
      {
        byte relays_state = check_current_relState();
        if (inPin == SW_UP)
        {
          if (relays_state != WIN_UP)
          {
            makeSwitch(WIN_UP);
            lastState = state;
          }
        }
        else if (inPin == SW_DOWN)
        {
          if (relays_state != WIN_DOWN)
          {
            makeSwitch(WIN_DOWN);
            lastState = state;
          }
        }
        else
        {
          if (relays_state != WIN_STOP)
          {
            makeSwitch(WIN_STOP);
            lastState = !SW_PRESSED;
          }
        }
      }
      else
      {
        makeSwitch(WIN_STOP);
        lastState = !SW_PRESSED;
      }
    }
  }
}
void allOff()
{
  digitalWrite(REL_UP, !RELAY_ON);
  digitalWrite(REL_DOWN, !RELAY_ON);
  delay(change_dir_delay);
}
void makeSwitch(byte state)
{
  Serial.write(state); // 10 + state - notify to ESP
  state = state % 10;

  if (state == WIN_STOP) /* Stop */
  {
    allOff();
  }
  else if (state == WIN_UP) /* Up */
  {
    allOff();
    digitalWrite(REL_UP, RELAY_ON);
  }
  else if (state == WIN_DOWN) /* DOWN */
  {
    allOff();
    digitalWrite(REL_DOWN, RELAY_ON);
  }
  else
  {
    allOff();
  }
}
void errorProtection()
{
  if (digitalRead(REL_UP) == RELAY_ON && digitalRead(REL_DOWN) == RELAY_ON)
  {
    makeSwitch(WIN_STOP);
  }
  if (digitalRead(SW_UP) == SW_PRESSED && digitalRead(SW_DOWN) == SW_PRESSED)
  {
  }
}
void read_allInputs()
{
  readInput(SW_UP, REL_UP, swUp_lastState);
  readInput(SW_DOWN, REL_DOWN, swDown_lastState);
}

void readSerial()
{
  byte x = -2;
  if (Serial.available() > 0)
  {
    x = Serial.read();
    if (x % 10 == WIN_STOP || x % 10 == WIN_UP || x % 10 == WIN_DOWN)
    {
      makeSwitch(x);
    }
  }
}
void setup()
{
  start_gpio();
  Serial.begin(115200);
}
void loop()
{
  read_allInputs();
  // errorProtection();
  readSerial();
  delay(loop_delay);
}
