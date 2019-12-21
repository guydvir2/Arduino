int outpin1 = D2;
int outpin2 = D3;
bool state1 = LOW;
bool state2 = LOW;
void setup() {
  // put your setup code here, to run once:
  pinMode(outpin1, OUTPUT);
  pinMode(outpin2, OUTPUT);
  digitalWrite(outpin1, state1);
  digitalWrite(outpin2, state2);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  state1 = !state1;
  state2 = !state2;

  digitalWrite(outpin1, state1);
  digitalWrite(outpin2, state2);

  delay(2000);

}
