int Pin1 = D1;
// int Pin2 = 7;
bool lastPin1=false;
bool lastPin2=false;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(Pin1, INPUT);
// pinMode(Pin2, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(Pin1)== true && lastPin1 == false){
    Serial.print("Pin1=");
    Serial.println(digitalRead(Pin1));
    lastPin1=true;
  }
  else if(digitalRead(Pin1)==false&& lastPin1 == true){
    lastPin1 = false;
    Serial.print("Pin1=");
    Serial.println(digitalRead(Pin1));
  }
  // if (digitalRead(Pin2)== true && lastPin2 == false){
  //   Serial.print("Pin2=");
  //   Serial.println(digitalRead(Pin2));
  //   lastPin2=true;
  // }
  // else if(digitalRead(Pin2)==false&& lastPin2 == true){
  //   lastPin2 = false;
  //   Serial.print("Pin2=");
  //   Serial.println(digitalRead(Pin2));
  // }

  delay(100);

}
