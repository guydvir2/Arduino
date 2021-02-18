#define REL_DOWN 2 /* OUTUPT to relay device */
#define REL_UP 3   /* OUTUPT to relay device */
#define SW_UP 4    /* Switch INPUT to Arduino */
#define SW_DOWN 5  /* Switch INPUT to Arduino */

#define SW_PRESSED LOW
#define RELAY_ON HIGH

#define VER "Arduino_v0.3"
#define MQTT_OFFSET 10  /* Actions made by ESP8266 are +10 by value */
#define QUERY_OFFSET 40 /* Query about State are +40 by value */
enum sys_states : byte
{
  WIN_STOP,
  WIN_UP,
  WIN_DOWN,
  WIN_ERR,
  QUERY,
  RESET,
  BOOT
};

const byte change_dir_delay = 20; //ms
const byte debounce_delay = 20;   //ms
const byte loop_delay = 10;       //ms

bool swUp_lastState = !SW_PRESSED;
bool swDown_lastState = !SW_PRESSED;

void start_gpio()
{
  pinMode(SW_UP, INPUT_PULLUP);
  pinMode(SW_DOWN, INPUT_PULLUP);

  pinMode(REL_UP, OUTPUT);
  pinMode(REL_DOWN, OUTPUT);

  allOff();
}
void allOff()
{
  digitalWrite(REL_UP, !RELAY_ON);
  digitalWrite(REL_DOWN, !RELAY_ON);
  delay(change_dir_delay);
}
void init_reset()
{
}
void send_msg(byte &msg)
{
  Serial.write(msg);
}
byte getWin_state()
{
  bool relup = digitalRead(REL_UP);
  bool reldown = digitalRead(REL_DOWN);

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

  switch (state)
  {
  case WIN_STOP:
    allOff();
    break;
  case WIN_UP:
    allOff();
    digitalWrite(REL_UP, RELAY_ON);
    break;
  case WIN_DOWN:
    allOff();
    digitalWrite(REL_DOWN, RELAY_ON);
    break;
  default:
    allOff();
    break;
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
    init_reset();
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
      if (state == SW_PRESSED)
      {
        byte relays_state = getWin_state();
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
        // else
        // {
        //   if (relays_state != WIN_STOP)
        //   {
        //     makeSwitch(WIN_STOP);
        //     lastState = !SW_PRESSED;
        //   }
        // }
      }
      else
      {
        makeSwitch(WIN_STOP);
        lastState = !SW_PRESSED;
      }
    }
  }
}
void Serial_cmd_callbacks(byte &x)
{
  if ((x >= WIN_STOP && x <= WIN_DOWN) || (x >= WIN_STOP + MQTT_OFFSET && x <= WIN_DOWN + MQTT_OFFSET))
  {
    makeSwitch(x); /* x={0,1,2,10,11,12}*/
  }
  else if (x == QUERY)
  {
    Serial.write(getWin_state()+ QUERY_OFFSET); /* {40,41,42,43} */
  }
  else if (x == RESET)
  {
    send_msg(x);
    delay(10);
    init_reset();
  }
  else
  {
    Serial.write(9);
  }
}
void readSerial()
{
  /* avoid burst serial input*/
  static long last_msg = 0;
  int min_time_msg = 20; //ms between messages

  if (Serial.available() > 0 && millis() - last_msg > min_time_msg)
  {
    last_msg = millis();
    byte x = Serial.read();
    Serial_cmd_callbacks(x);
  }
}
void setup()
{
  start_gpio();
  Serial.begin(9600);
  Serial.write(BOOT);
}
void loop()
{
  readInput(SW_UP, swUp_lastState);
  readInput(SW_DOWN, swDown_lastState);
  errorProtection();
  readSerial();
  delay(loop_delay);
}
