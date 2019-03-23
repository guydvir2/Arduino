float metalDetected;
int readValue;
int sensorPin = A1;
int ledPin= 3;

void setup() {
  Serial.begin(9600);
  Serial.println(" Hi !");
  pinMode(ledPin, OUTPUT);
}

void loop() {
  readValue = analogRead(sensorPin);
  metalDetected = (float) readValue * 100 / 1024.0;

  if (readValue > 250
  ) {
//    Serial.println("Metal is Detected");
    Serial.print("Metal is Proximited = ");
    Serial.print(metalDetected);
    Serial.println("%");

    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}
