int sensorPin = D8;

void setup() {
  pinMode(sensorPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
Serial.println(digitalRead(sensorPin));
delay(500);
}
