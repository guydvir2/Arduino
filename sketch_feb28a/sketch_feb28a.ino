int pin1=A0;
int pin2=A1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Pin1: [");
  Serial.print(analogRead(pin1));
  Serial.print("]; Pin2: [");
  Serial.print(analogRead(pin2));
  Serial.println("]");

  delay(500);

}
