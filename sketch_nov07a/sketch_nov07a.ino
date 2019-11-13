int SensorPin = D7;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(SensorPin, INPUT);
  Serial.println("BEGIN");

}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(SensorPin) == 1) {
    Serial.println("DETECTION");
  }
  else {
    Serial.println("WAIT");
  }

  //Serial.println(digitalRead(SensorPin));


  delay(1000);

}
