int pin = D7;

void setup() {
Serial.begin(9600);
Serial.println("\nStarT");
pinMode(pin, INPUT_PULLUP);


}

void loop() {
  Serial.print("SW: ");
  Serial.println(digitalRead(pin));
  delay(100);


}
