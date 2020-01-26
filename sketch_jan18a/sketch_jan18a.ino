int pwmPin = D3;
int buttonPin = D7;
int sensorPin = D4;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pwmPin, INPUT);
  pinMode (buttonPin, INPUT_PULLUP);
  pinMode(sensorPin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  // for (int a=0;  a<=1023; a+=20){
  //   analogWrite(pwmPin, a);
  //   Serial.println(a);
  //   if (digitalRead(buttonPin)==LOW){
  //     Serial.println("LOW");

  //   }
  //   else{
  //     Serial.println("HIGH");
  //   }
  //   delay(10);
  // }
  if (digitalRead(buttonPin)==LOW){
      Serial.println("LOW");

    }
    else{
      Serial.println("HIGH");
    }
    if (digitalRead(sensorPin)==LOW){
      Serial.println("snLOW");

    }
    else{
      Serial.println("snHIGH");
    }

    delay(500);
}
