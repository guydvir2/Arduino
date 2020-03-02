const int pin0=D8;
const int pin1=D7;
// const int pin2=A2;
// const int pin3=A3;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
pinMode(pin0, INPUT);
pinMode(pin1, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Pin0: [");
  Serial.print(digitalRead(pin0));
  Serial.print("]; Pin1: [");
  Serial.print(digitalRead(pin1));
  // Serial.print("]; Pin2: [");
  // Serial.print(analogRead(pin2));
  // Serial.print("]; Pin3: [");
  // Serial.print(analogRead(pin3));
  Serial.println("]");

  delay(500);

}
