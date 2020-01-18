void setup() {
  Serial.begin(74880);
  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println("Started.");
  delay(5000);
  Serial.println("Going to sleep.");
  ESP.deepSleep(30e6);
}

void loop() { }
