void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(D7, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(D7));
  delay(200);
}
