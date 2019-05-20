int gatePin=10;

void setup() {
  // put your setup code here, to run once:
  pinMode(gatePin,OUTPUT);
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  // for (int x=0; x<=255; x=x+50){
    int x=0;
    analogWrite(gatePin, x);
    Serial.println(x);
    delay(2000);
    x=255;
    analogWrite(gatePin, x);
    Serial.println(x);
    delay(2000);
  // }

}
