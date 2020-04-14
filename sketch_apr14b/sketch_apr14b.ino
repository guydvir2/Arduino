int senspin = D7;
bool lastState;
bool IS_DETECT = HIGH;
//int buzzer = D5; //Buzzer control port, default D5
//
//int freq[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976, 2093}; //Note name: C6 D6 E6 F6 G6 A6 B6 C7   http://newt.phys.unsw.edu.au/jw/notes.html
//String note[] = {"C6", "D6", "E6", "F6", "G6", "A6", "B6", "C7"};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(senspin, INPUT);
//  pinMode(buzzer, OUTPUT);
//  digitalWrite(buzzer, LOW);
  Serial.println("\n\n30 sec delay");
  delay(30000);
  Serial.println("Start");
  lastState = digitalRead(senspin);
}

void loop() {
  // put your main code here, to run repeatedly:
  bool sensval = digitalRead(senspin);
  if (sensval == IS_DETECT && lastState == !IS_DETECT) {
    Serial.println("Detect");
    lastState = !lastState;
  }
  else if ( sensval == !IS_DETECT && lastState == IS_DETECT) {
    Serial.println("EndDetect");
    lastState = !lastState;

  }
  //Serial.println(sensval);
  delay(100);
}
