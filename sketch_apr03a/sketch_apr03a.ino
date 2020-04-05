int sensPin=3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(sensPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println(digitalRead(sensPin));
// digitalWrite(sensPin,1);
//   delay(1000);
  digitalWrite(sensPin,0);
  delay(1000);

}
