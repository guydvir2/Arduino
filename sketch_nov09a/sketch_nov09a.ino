int enabler=D3;
int Vin = D2;
void setup() {
  // put your setup code here, to run once:
  pinMode (enabler, OUTPUT);
  pinMode (Vin, OUTPUT);
  Serial.begin(9600);

  // digitalWrite(Vin, HIGH);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(enabler, !digitalRead(enabler));
  Serial.println(digitalRead(enabler));

  delay(2000);

}
