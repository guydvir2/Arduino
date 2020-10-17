int sensPin = D2;
bool detect = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BEGIN");
  pinMode(sensPin, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(detect==false && digitalRead(sensPin)==true){
    Serial.println("Detect");
    detect=true;
  }
  else if (detect==true && digitalRead(sensPin)==false){
    detect=false;
    Serial.println("END_Detect");
  }
  delay(100);

}
