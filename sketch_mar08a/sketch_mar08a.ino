#define in1 D2
#define out1 D6
#define out2 D5

bool stateFlag = false;

void setup() {
  Serial.begin(9600);
  pinMode(in1, INPUT_PULLUP);
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);

  digitalWrite(out1, HIGH);
  digitalWrite(out2, LOW);

  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!digitalRead(in1)) {
    Serial.println("IN1");
    stateFlag = !stateFlag;
    if (stateFlag ) {
      digitalWrite(out1, LOW);
      digitalWrite(out2, HIGH);
    }
    else {
      digitalWrite(out1, HIGH);
      digitalWrite(out2, LOW);
    }
    while (!digitalRead(in1)) {
      delay(50);
    }
  }

  digitalWrite(out1,!digitalRead(out1));
  digitalWrite(out2,!digitalRead(out2));

  delay(100);
}
