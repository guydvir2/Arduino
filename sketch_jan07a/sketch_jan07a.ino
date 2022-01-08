int sensorPin = D1;
const bool DETECT = HIGH;

void checkDetection() {
  static bool inDetection = false;
  if (digitalRead(sensorPin) == DETECT && inDetection == false) {
    Serial.println("Detection!");
    inDetection = true;
  }
  else if (digitalRead(sensorPin) == !DETECT && inDetection == true) {
    Serial.println("Detection- Ended");
    inDetection = false;

  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nStart");

}

void loop() {
  // put your main code here, to run repeatedly:
  checkDetection();
  delay(200);

}
