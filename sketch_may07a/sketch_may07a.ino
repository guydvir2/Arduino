bool volatile state;
bool volatile state2;
bool last_state =true;
bool last_state2;
int pin=3;
int pin2=2;
int out=4;
int out2=5;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pin, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);
  pinMode(out, OUTPUT);
  pinMode(out2, OUTPUT);

  // attachInterrupt(digitalPinToInterrupt(pin), ISR_1, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(pin2), ISR_2, CHANGE);
}

void ISR_1(){
  state=!state;
}
void ISR_2(){
  state2=!state2;
}

void loop() {

  // last_state=!last_state;



  digitalWrite(out,digitalRead(pin));
  digitalWrite(out2,digitalRead(pin2));

  delay(1000);

  // if(last_state != state){
  //   last_state=state;
  //   Serial.print("1: ");
  //   Serial.println(digitalRead(pin));
  // }
  // if(last_state2 != state2){
  //   last_state2=state2;
  //   Serial.print("2: ");
  //   Serial.println(digitalRead(pin2));
  // }
  //
  //
  // Serial.print("#1: ");
  // Serial.println(digitalRead(pin));
  //
  // Serial.print("#2: ");
  // Serial.println(digitalRead(pin2));
  //
  // delay(2000);

}
