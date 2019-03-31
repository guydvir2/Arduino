void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
pinMode(D6,OUTPUT);
pinMode(D7,OUTPUT);
pinMode(D8,OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(D6, true);
  Serial.println("D6 TRUE");
  delay(2000);
  digitalWrite(D6, false);
  Serial.println("D6 FALSE");
  delay(2000);
  digitalWrite(D7, true);
  Serial.println("D7 TRUE");
  delay(2000);
  digitalWrite(D7, false);
  Serial.println("D7 FALSE");
  delay(2000);
  digitalWrite(D8, true);
  Serial.println("D8 TRUE");
  delay(2000);
  digitalWrite(D8, false);
  Serial.println("D8 FALSE");
  delay(2000);

}
