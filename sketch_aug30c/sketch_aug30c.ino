int sesPin = D8;

void setup() {
  Serial.begin(9600);
  pinMode(sesPin, INPUT);
}

void loop() {
  Serial.println(digitalRead(sesPin));
  delay(300);

}
