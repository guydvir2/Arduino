#define LED_OFF LOW
#define LED_ON !LED_OFF
#define BUT_PRESSED LOW

const byte ledPins[] = {0, 1, 2};
const byte buttonPin = 3;
const byte buzPin = 4;
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
  const byte x = 10;
  byte c = 0;
  while (c < 3)
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(ledPins[i], LED_ON);
      delay(200);
    }
    delay(500);
    for (int i = 2; i >= 0; i--)
    {
      digitalWrite(ledPins[i], LED_OFF);
      delay(200);
    }
    c++;
    make_buz();
  }
}
void make_buz()
{
  digitalWrite(buzPin, HIGH);
  delay(40);
  digitalWrite(buzPin, LOW);
}
void read_button()
{
  if (digitalRead(buttonPin) == BUT_PRESSED)
  {
    delay(50);
    if (digitalRead(buttonPin) == BUT_PRESSED)
    {
      if (button_state + 1 <= tot_states)
      {
        button_state++;
      }
      else
      {
        button_state = 0;
      }
      make_buz();
      delay(50);
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
