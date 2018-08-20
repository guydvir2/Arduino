

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

}
int test(int y) {
  int x = 0;
  x = x + y;
  return x;
}
void loop() {
  for (int m = 0; m < 100; m++) {
    Serial.println(test(m));
    delay(1000);


  }
  // put your main code here, to run repeatedly:

}
