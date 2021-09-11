int pin=10;
void setup() {
  // put your setup code here, to run once:
  Serial1.begin(115200);
  Serial1.println("BEGIN");
  pinMode(pin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(pin));
  delay(100);

}
