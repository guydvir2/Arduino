/* Sender Side */

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  static long last = 0;
  static int x = 0;
  while (millis() - last > 2000) {
    last = millis();
    Serial.write(x);
    if (x < 2) {
      x++;
    }
    else {
      x = 0;
    }
//    Serial.println(x);
  }
}
