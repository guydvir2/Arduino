int ioPin = 14;
int relayPin = 12;
int ledPin = 13;
void setup()
{
  pinMode(ioPin, INPUT_PULLUP); // Initialize the BUILTIN_LED pin as an output
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
}

void loop()
{
  Serial.println(digitalRead(ioPin));
  digitalWrite(relayPin, digitalRead(ioPin));
  digitalWrite(ledPin, digitalRead(ioPin));
  delay(500);
}