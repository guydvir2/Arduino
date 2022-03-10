int outputPin = D1;
int inputPin = D5;


void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Start");
  pinMode(outputPin, OUTPUT);
//  pinMode(inputPin, INPUT_PULLUP);
//    digitalWrite(outputPin, LOW);
  //analogWrite(outputPin,0);
}

void loop()
{
//    digitalWrite(outputPin, !digitalRead(outputPin));
//    Serial.println(digitalRead(outputPin));
//  for (int i = 0; i < 256; i++) {
//    analogWrite(outputPin, i);
//    delay(10);
//  }
//    digitalWrite(outputPin, HIGH);
//    delay(10);
//    digitalWrite(outputPin, LOW);
//    delay(1000);
Serial.println(digitalRead(D5));
}
