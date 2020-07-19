int pin=9;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pin, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println(digitalRead(pin));
delay(500);
}
