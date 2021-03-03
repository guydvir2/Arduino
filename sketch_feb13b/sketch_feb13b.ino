/* Recv side ESP8266*/
int pins[] = {0, 1, 2};
#define LED_OFF LOW
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int i = 0; i < 3; i++)
  {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LED_OFF);
  }
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(pins[i], !LED_OFF);
    delay(1000);
    digitalWrite(pins[i], LED_OFF);
    delay(1000);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    int x = Serial.read();
    switch (x)
    {
    case 0:
      digitalWrite(pins[0], !LED_OFF);
      digitalWrite(pins[1], LED_OFF);
      digitalWrite(pins[2], LED_OFF);
      break;
    case 1:
      digitalWrite(pins[0], LED_OFF);
      digitalWrite(pins[1], !LED_OFF);
      digitalWrite(pins[2], LED_OFF);
      break;
    case 2:
      digitalWrite(pins[0], LED_OFF);
      digitalWrite(pins[1], LED_OFF);
      digitalWrite(pins[2], !LED_OFF);
      break;
    default:
      digitalWrite(pins[0], LED_OFF);
      digitalWrite(pins[1], LED_OFF);
      digitalWrite(pins[2], LED_OFF);
    }
  }
}
