int outputPin = D6;


void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Start");
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);
}

void loop()
{
  digitalWrite(outputPin, !digitalRead(outputPin));
  delay(1000);
}
