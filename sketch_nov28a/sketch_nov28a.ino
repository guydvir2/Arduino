int inPin1 = D5;
int inPin2 = D6;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(inPin1, INPUT_PULLUP);
  pinMode(inPin2, INPUT_PULLUP);
  Serial.println("Start");

}

void loop() {
  // put your main code here, to run repeatedly:
Serial.print(digitalRead(inPin1));
Serial.print("; ");
Serial.println(digitalRead(inPin2));
delay(500);
}
