int buttonPin = 7;
int redLed = 5;
int orangeLed = 6;
int greenLed = 4;
const bool ledON = HIGH;
int numPress = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redLed, OUTPUT);
  pinMode(orangeLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  digitalWrite(redLed, ledON);
  digitalWrite(orangeLed, ledON);
  digitalWrite(greenLed, ledON);
  delay(1000);
  digitalWrite(redLed, !ledON);
  digitalWrite(orangeLed,!ledON);
  digitalWrite(greenLed, !ledON);
  delay(1000);
  digitalWrite(redLed, ledON);
  digitalWrite(orangeLed, ledON);
  digitalWrite(greenLed, ledON);
  delay(1000);
  digitalWrite(redLed, !ledON);
  digitalWrite(orangeLed,!ledON);
  digitalWrite(greenLed, !ledON);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(buttonPin) == 0) {
    delay(50);
    if (digitalRead(buttonPin) == 0) {
      if (numPress < 3) {
        numPress++;
      }
      else {
        numPress = 0;
      }
    }
  }
  if (numPress == 0) {
    digitalWrite(redLed, !ledON);
    digitalWrite(orangeLed, !ledON);
    digitalWrite(greenLed, !ledON);
  }
  else if (numPress == 1) {
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, !ledON);
    digitalWrite(greenLed, !ledON);
  }
  else if (numPress == 2) {
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, ledON);
    digitalWrite(greenLed, !ledON);
  }
  else if (numPress == 3) {
    digitalWrite(redLed, ledON);
    digitalWrite(orangeLed, ledON);
    digitalWrite(greenLed, ledON);
  }
  delay(200);


}
