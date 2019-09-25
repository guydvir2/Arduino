int pin = D1;
int x=255;

void setup() {
  // put your setup code here, to run once:
pinMode(pin, INPUT_PULLUP);
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
  delay(200);
  Serial.println(digitalRead(pin));

}
