int buttonPin = 7;
int redLed = 5;
int orangeLed = 6;
int greenLed = 4;
const bool ledON = HIGH;
int numPress = 0;

void blinkAll(int x = 3, int del = 1000)
{
  for (int i = 0; i < x; i++)
  {
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
  Serial.print("color: ");
  Serial.println(color);
  Serial.print("pressCount: ");
  Serial.println(pressCount);

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

  blinkAll(5, 200);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (digitalRead(buttonPin) == 0)
  {
    delay(50);
    if (digitalRead(buttonPin) == 0)
    {
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
  // blinkOneColor(redLed, 1, numPress);
  // blinkOneColor(orangeLed, 2, numPress);
  // blinkOneColor(greenLed, 3, numPress);

  // if (numPress == 0)
  // {
  //   digitalWrite(redLed, !ledON);
  //   digitalWrite(orangeLed, !ledON);
  //   digitalWrite(greenLed, !ledON);
  // }
  // else if (numPress == 1)
  // {
  //   digitalWrite(redLed, ledON);
  //   digitalWrite(orangeLed, !ledON);
  //   digitalWrite(greenLed, !ledON);
  // }
  // else if (numPress == 2)
  // {
  //   digitalWrite(redLed, ledON);
  //   digitalWrite(orangeLed, ledON);
  //   digitalWrite(greenLed, !ledON);
  // }
  // else if (numPress == 3)
  // {
  //   digitalWrite(redLed, ledON);
  //   digitalWrite(orangeLed, ledON);
  //   digitalWrite(greenLed, ledON);
  // }
  delay(200);
}
