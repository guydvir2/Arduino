int pinS = 2;
int pinBuzzer = 11;
void setup() {
  pinMode(pinS, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);
  Serial.begin(9600);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

  if (digitalRead(pinS) == 0){
    Serial.println("BEEP");
    digitalWrite(pinBuzzer, 1);
    delay(300);
    digitalWrite(pinBuzzer, 0);
    delay(300);
  }

  delay(50);
  
}
