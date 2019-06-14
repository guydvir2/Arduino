int pin=10;
void setup() {
  // put your setup code here, to run once:
pinMode(pin, OUTPUT);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(pin,0);
  Serial.println(LOW);
  delay(5000);
  analogWrite(pin,255);
  Serial.println(HIGH);
  delay(5000);

}
