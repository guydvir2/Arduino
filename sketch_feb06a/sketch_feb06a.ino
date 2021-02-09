int relPin=8;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("HI");
  pinMode(relPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(relPin, HIGH);
  Serial.println("HIGH");
  delay(2000);
  digitalWrite(relPin, LOW);
  Serial.println("LOW");
  delay(2000);

}
