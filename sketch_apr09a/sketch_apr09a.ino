int input0 = 1;
int input1 = 3;
int output0 = 0;
int output1 = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(input0, INPUT_PULLUP);
  pinMode(input1, INPUT_PULLUP);
  pinMode(output0, OUTPUT);
  pinMode(output1, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(input0, !digitalRead(output0));
  digitalWrite(input1, !digitalRead(output1));
  delay(2000);
  Serial.println("LOOP");
}
