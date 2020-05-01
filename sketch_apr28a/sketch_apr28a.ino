int pins[] = {2, 3};
bool last_boot[2];

void setup() {
  // put your setup code here, to run once:
  for (int a = 0; a < 2; a++) {
    pinMode(pins[a], INPUT);
  }
  Serial.begin(9600);
  delay(60000);
  Serial.println("START");

}

void loop() {
  // put your main code here, to run repeatedly:
  bool curr_boot[2];

  for (int a = 0; a < 2; a++) {
    curr_boot[a] = digitalRead(pins[a]);
    if (curr_boot[a] != last_boot[a]) {
      last_boot[a] = curr_boot[a];
      Serial.print("Sernsor #");
      Serial.print(a);
      Serial.print(": is now ");
      Serial.println(curr_boot[a]);
    }
    delay(500);
  }


}
