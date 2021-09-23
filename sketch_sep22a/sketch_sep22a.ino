int but_pin = D1;
int led_pin = D2;
bool ledState=false;
ICACHE_RAM_ATTR void action_cb() {
  ledState=!ledState;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start");
  pinMode(but_pin, INPUT_PULLUP);
  pinMode(led_pin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(but_pin), action_cb, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  digitalWrite(led_pin,ledState);

}
