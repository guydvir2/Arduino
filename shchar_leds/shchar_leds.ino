int redPin = 5;
int yellowPin = 6;
int greenPin = 7;

int greenButton = 2;
int redButton = 3;
int yellowButton = 4;

//#define redPin 1;
//#define yellowPin D3;
//#define greenPin D4;
//
//#define greenButton D5;
//#define redButton D6;
//#define yellowButton D7;

bool greenFlag = false;
bool yellowFlag = false;
bool redFlag = false;


void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
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
    Serial.println("Green");
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
