int ledPin = 13;
void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}
void loop()
{
  digitalWrite(ledPin, HIGH);
  //Serial.println(digitalRead(ledPin));
  Serial.println(1,"guy");

  delay(200);
  digitalWrite(ledPin, LOW);
  delay(200);
}

