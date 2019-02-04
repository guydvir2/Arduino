int redPin = 13;
int yellowPin = 12;
int greenPin = 11;

int greenButton = 8;
int redButton = 10;
int yellowButton = 9;

bool greenFlag = false;
bool yellowFlag = false;
bool redFlag = false;


void setup() {
  // put your setup code here, to run once:
  //  Serial.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  pinMode(greenButton, INPUT_PULLUP);
  pinMode(redButton, INPUT_PULLUP);
  pinMode(yellowButton, INPUT_PULLUP);


  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);

}

void loop() {
  // put your main code here, to run repeatedly:
//  digitalWrite(redPin, HIGH);
//  delay(100);
//  digitalWrite(greenPin, HIGH);
//  delay(200);
//  digitalWrite(yellowPin, HIGH);
//  delay(300);
//  digitalWrite(redPin, LOW);
//  delay(10);
//  digitalWrite(greenPin, LOW);
//  delay(20);
//  digitalWrite(yellowPin, LOW);
//  delay(30);



  //  Part 2
  //##########
  if (digitalRead(greenButton) == LOW) { // button is pressed
    digitalWrite(greenPin, HIGH);
  }
  else { // button is released
    digitalWrite(greenPin, LOW);
  }

  if (digitalRead(redButton) == LOW) { // button is pressed
    digitalWrite(redPin, HIGH);
  }
  else { // button is released
    digitalWrite(redPin, LOW);
  }

  if (digitalRead(yellowButton) == LOW) { // button is pressed
    digitalWrite(yellowPin, HIGH);
  }
  else { // button is released
    digitalWrite(yellowPin, LOW);
  }

// Part 3
//++++++++

//  if (digitalRead(greenButton) == LOW) { // button is pressed
//    digitalWrite(greenPin, !greenFlag);
//    greenFlag = !greenFlag;
//    delay(1000);
//  }


}
