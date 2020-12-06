
int sensPin = D7;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(sensPin, INPUT_PULLUP);
  Serial.println("start!");

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(sensPin));
  delay(500);

}
