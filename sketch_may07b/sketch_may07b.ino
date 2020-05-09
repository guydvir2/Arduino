int pin0 = 1;
int pin1 = 7;
bool lastPin0 = false;
bool lastPin1 = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pin0, INPUT);
  pinMode(pin1, INPUT);

  Serial.println("wait 30 sec");
  delay(30000);
}

void loop() {
  static bool msg = false;
  // put your main code here, to run repeatedly:
  if (digitalRead(pin0) != lastPin0) {
    lastPin0 = !lastPin0;
    Serial.print("Detector #0 is: ");
    Serial.println(lastPin0);
  }
  if (digitalRead(pin1) != lastPin1) {
    lastPin1 = !lastPin1;
    Serial.print("Detector #1 is: ");
    Serial.println(lastPin1);
  }

//  if (lastPin0 && lastPin1 && msg == false) {
//    Serial.println("Confirmed detection");
//    msg = true;
//  }
//  else {
//    msg = false;
//
//  }
  delay(100);

}
