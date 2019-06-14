void func (int a, int b=0){
  Serial.println(a);
  Serial.println(b);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  func(3);

}

void loop() {
  // put your main code here, to run repeatedly:

}
