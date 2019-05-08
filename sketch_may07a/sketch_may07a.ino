bool volatile state;
bool volatile state2;
bool last_state;
bool last_state2;
int pin=3;
int pin2=2;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Serial.println(ptr);
  pinMode(pin, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pin), ISR_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(pin2), ISR_2, RISING);
}

void ISR_1(){
  state=!state;
}
void ISR_2(){
  state2=!state2;
}

void loop() {
  if(last_state != state){
    last_state=state;
    Serial.print("1: ");
    Serial.println(digitalRead(pin));
  }
  if(last_state2 != state2){
    last_state2=state2;
    Serial.print("2: ");
    Serial.println(digitalRead(pin2));
  }


  Serial.print("#1: ");
  Serial.println(digitalRead(pin));

  Serial.print("#2: ");
  Serial.println(digitalRead(pin2));

  delay(2000);

}
