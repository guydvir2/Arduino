int pin1 = 2;
int pin2 = 3;
bool last = false;
void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(pin1, INPUT);
  //  digitalWrite(pin2, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  bool stat = digitalRead(pin1);
  if (stat != last ) {
    last = stat;
    if (stat) {
      Serial.println("detection");
    }
    else {
      Serial.println("END detection");
    }
  }
  delay(500);
}
