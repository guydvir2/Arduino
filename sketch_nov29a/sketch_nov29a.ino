const byte buttonPin = 4;
const byte redLed = 8;
const byte orangeLed = 9;
const byte greenLed = 10;
const byte buzzerPin = 3;
const bool ledON = HIGH;

byte numPress = 0;

void beep(int f = 200, int dur = 50)
{
  analogWrite(buzzerPin, f);
  delay(dur);
  analogWrite(buzzerPin, 0);
}
void blinkAll(int x = 3, int del = 1000)
{
  for (int i = 0; i < x; i++)
  {
    beep(100, 20);
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, ledON);
    digitalWrite(greenLed, ledON);
    delay(del);
    digitalWrite(redLed, !ledON);
    digitalWrite(orangeLed, !ledON);
    digitalWrite(greenLed, !ledON);
    delay(del);
  }
}
void blinkOneColor(int &color, int condition, int pressCount)
{
  if (pressCount >= condition)
  {
    digitalWrite(color, !digitalRead(color));
  }
  else
  {
    digitalWrite(color, !ledON);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redLed, OUTPUT);
  pinMode(orangeLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  analogWrite(buzzerPin, 200);
  delay(10);
  analogWrite(buzzerPin, 0);

  blinkAll(5, 200);
}

void loop()
{
  if (digitalRead(buttonPin) == 0)
  {
    delay(50);
    if (digitalRead(buttonPin) == 0)
    {
      beep(100, 50);
      if (numPress < 5)
      {
        numPress++;
      }
      else
      {
        numPress = 0;
      }
    }
  }
  switch (numPress)
  {
  case 0:
    digitalWrite(redLed, !ledON);
    digitalWrite(orangeLed, !ledON);
    digitalWrite(greenLed, !ledON);
    break;
  case 1:
    digitalWrite(redLed, !digitalRead(redLed));
    digitalWrite(orangeLed, !ledON);
    digitalWrite(greenLed, !ledON);
    break;
  case 2:
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, !digitalRead(orangeLed));
    digitalWrite(greenLed, !ledON);
    break;
  case 3:
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, ledON);
    digitalWrite(greenLed, !digitalRead(greenLed));
    break;
  case 4:
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, ledON);
    digitalWrite(greenLed, ledON);
    break;
  case 5:
    digitalWrite(redLed, !digitalRead(redLed));
    digitalWrite(orangeLed, !digitalRead(orangeLed));
    digitalWrite(greenLed, !digitalRead(greenLed));
    break;

    // default:
    //   break;
  }
  delay(100);
}
