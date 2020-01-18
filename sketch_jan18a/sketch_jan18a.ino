void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("bat is: ");
  Serial.print(analogRead(36));//*1.1/4095*3.3);
  Serial.println("[v]");
    Serial.print("SolarPanel is: ");
  Serial.print(analogRead(39));//*1.1/4095*3.3);
  Serial.println("[v]");

}

void loop() {
  // put your main code here, to run repeatedly:

}
