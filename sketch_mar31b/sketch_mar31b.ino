void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
pinMode(D1,OUTPUT);


}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(D1, true);
  Serial.println("D1 TRUE");
  delay(200);
  digitalWrite(D1, false);
  Serial.println("D1 FALSE");
  delay(200);

}
