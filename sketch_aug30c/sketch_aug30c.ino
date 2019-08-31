int MosfetPin = D3;
int switchPin = D7;

void setup() {
  pinMode(D3, OUTPUT);
  pinMode(D7, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
digitalWrite(D3, digitalRead(D7));
Serial.println(digitalRead(D7));
delay(500);
}
