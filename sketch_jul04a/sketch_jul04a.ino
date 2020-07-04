int PWMpin=D1;
int x=0;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(PWMpin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
analogWrite(PWMpin,0);
delay(5000);
analogWrite(PWMpin,1023);
delay(5000);



}
