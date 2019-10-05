int swPin = D8;
int ledPin = D7;
void setup() {
  // put your setup code here, to run once:
pinMode(swPin, INPUT_PULLUP);
// pinMode(ledPin, OUTPUT);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // if (x <=255){
  //   x=x+40;
  // }
  // else{
  //   x=15;
  // }
  // analogWrite(D4,x);
  delay(1000);
  Serial.print("wdPin: ");
  Serial.println(digitalRead(swPin));
  // Serial.print("ledPin: ");
  //   Serial.println(digitalRead(ledPin));
  //   digitalWrite(ledPin,!digitalRead(ledPin));

}
