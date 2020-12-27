int iopin = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(iopin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 1023; i++) {
    analogWrite(iopin, i);
    delay(5);
  }
  delay(1000);

}
