int gpios[] = {4, 5, 6, 7};
const byte s = 4;
const bool RelayON = HIGH;
void relay_loop(int interval = 500, int del = 500) {
  for (int i = 0; i < s; i++) {
    Serial.print("#");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(gpios[i]);
    pinMode(gpios[i], OUTPUT);
    digitalWrite(gpios[i], RelayON);
    delay(interval);
    digitalWrite(gpios[i], !RelayON);
    delay(interval);
  }
  delay(del);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start!");
}

void loop() {
  // put your main code here, to run repeatedly:
  relay_loop(5000,100);
}
