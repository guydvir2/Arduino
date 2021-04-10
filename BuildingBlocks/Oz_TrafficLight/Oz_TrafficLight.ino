#define LED_OFF LOW
#define LED_ON !LED_OFF
#define BUT_PRESSED LOW

/* Pins 4,5 may fail boot if pulled up!*/
const byte ledPins[] = {2, 3, 4};
const byte buttonPin = 0;
const byte buzPin = 1;
const byte tot_states = 7;
byte button_state = 0;

void start_gpio()
{
  for (int i = 0; i < 3; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LED_OFF);
  }
  pinMode(buttonPin, INPUT_PULLUP);
}
void led_boot_test()
{
  const byte x = 3;
  byte c = 0;
  while (c < x)
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(ledPins[i], LED_ON);
      delay(100);
    }
    delay(300);
    for (int i = 2; i >= 0; i--)
    {
      digitalWrite(ledPins[i], LED_OFF);
      delay(100);
    }
    c++;
    make_buz();
  }
}
void make_buz()
{
  analogWrite(buzPin, 80);
  delay(100);
  analogWrite(buzPin, 0);
}
void read_button()
{
  static unsigned long lastPress = 0;
  const int time_between_press = 1000; // ms
  if (digitalRead(buttonPin) == BUT_PRESSED)
  {
    delay(50);
    if (digitalRead(buttonPin) == BUT_PRESSED)
    {
      if (millis() - lastPress >= time_between_press)
      {
        lastPress = millis();
        if (button_state + 1 <= tot_states)
        {
          button_state++;
        }
        else
        {
          button_state = 0;
        }
        make_buz();
      }
    }
  }
}
void traffic_light(byte arr[])
{
  for (int i = 0; i < 3; i++)
  {
    if (arr[i] <= 1)
    {
      digitalWrite(ledPins[i], arr[i]);
    }
    else
    {
      digitalWrite(ledPins[i], !digitalRead(ledPins[i]));
    }
  }
}

void lightup()
{
  byte t[3] = {0, 0, 0};
  switch (button_state)
  {
  case 0:
    t[0] = 0;
    t[1] = 0;
    t[2] = 0;
    break;
  case 1:
    t[0] = 2;
    t[1] = 0;
    t[2] = 0;
    break;
  case 2:
    t[0] = 1;
    t[1] = 0;
    t[2] = 0;
    break;
  case 3:
    t[0] = 1;
    t[1] = 2;
    t[2] = 0;
    break;
  case 4:
    t[0] = 1;
    t[1] = 1;
    t[2] = 0;
    break;
  case 5:
    t[0] = 1;
    t[1] = 1;
    t[2] = 2;
    break;
  case 6:
    t[0] = 1;
    t[1] = 1;
    t[2] = 1;
    break;
  case 7:
    t[0] = 2;
    t[1] = 2;
    t[2] = 2;
    break;
  }

  traffic_light(t);
}
void setup()
{
  start_gpio();
  led_boot_test();
}

void loop()
{
  read_button();
  lightup();
  delay(100);
}
