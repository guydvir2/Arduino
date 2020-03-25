const int pins[] = {0, 2, 15, 13};

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  for (int a = 0; a < 4; a++) {
    pinMode(pins[a], INPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("~~~~~~~~~~~~~~~~~~~~");

  for (int a = 0; a < 4; a++) {
    Serial.print("Sensor #");
    Serial.print(a);
    Serial.print(": ");
    Serial.println(analogRead(pins[a]));
  }
  Serial.println("++++++++++++END+++++++++++\n\n\n");

  delay(2000);

}
