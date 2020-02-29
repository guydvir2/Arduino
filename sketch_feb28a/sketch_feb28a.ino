const int pin0=A0;
const int pin1=A1;
const int pin2=A2;
// const int pin3=A3;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
pinMode(A0, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Pin0: [");
  Serial.print(analogRead(pin0));
  Serial.print("]; Pin1: [");
  Serial.print(analogRead(pin1));
  Serial.print("]; Pin2: [");
  Serial.print(analogRead(pin2));
  // Serial.print("]; Pin3: [");
  // Serial.print(analogRead(pin3));
  Serial.println("]");

  delay(500);

}
