int pin = 2;
void setup() {
  // put your setup code here, to run once:
  pinMode(pin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 255; i++) {
    analogWrite(pin, i);
    delay(100);
    Serial.println(i);
  }

  for (int i = 255; i > 0; i--) {
    analogWrite(pin, i);
    delay(100);
    Serial.println(i);
  }

}
