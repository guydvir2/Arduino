int pin0 = D3;
int pin1 = D4;

void setup() {
  // put your setup code here, to run once:
pinMode(pin0, OUTPUT);
pinMode(pin1, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pin0, !digitalRead(pin0));
  digitalWrite(pin1, !digitalRead(pin1));

  delay(5000);
  

}
