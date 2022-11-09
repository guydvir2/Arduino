int b1 = 34;
int b2 = 35;
int b3 = 36;
int b4 = 39;
int buzz = 4;
int rels[4] = {25, 26, 33, 32};
int buts[4] = {34, 35, 36, 39};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nBEGIN!");
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);
  pinMode(b3, INPUT_PULLUP);
  pinMode(b4, INPUT_PULLUP);
  pinMode(buzz, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(rels[i], OUTPUT);
    pinMode(buts[i], INPUT_PULLUP);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  char a[20];
  //  sprintf(a, "%d; %d; %d; %d", digitalRead(b1), digitalRead(b2), digitalRead(b3), digitalRead(b4));
  //  delay(200);
  //  Serial.println(a);
  //  digitalWrite(buzz,!digitalRead(buzz));

  for (int i = 0; i < 4; i++) {
    digitalWrite(rels[i], digitalRead(buts[i]));
  }

}
