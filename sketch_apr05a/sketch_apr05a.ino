int pin=D5;


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(pin,INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println(digitalRead(pin));
}
