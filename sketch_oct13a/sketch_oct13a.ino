int x1 = 2;
int x2 = 3;
int x3 = 4;
int x4 = 5;


//int &xp = x;
//int &xpp = xp;
//
//int *p;       // declare a pointer to an int data type
//int i = 5;
//int result = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start");
  pinMode(x1, INPUT_PULLUP);
  pinMode(x2, INPUT_PULLUP);
  pinMode(x3, INPUT_PULLUP);
  pinMode(x4, INPUT_PULLUP);

//  p = &i;       // now 'p' contains the address of 'i'
//  result = *p;  // 'result' gets the value at the address pointed by 'p'
//  // i.e., it gets the value of 'i' which is 5

  //  xp = 11;
  //  Serial.print("X=");
  //  Serial.println(x);
  //  Serial.print("Xp=");
  //  Serial.println(xp);
  //  Serial.print("Xpp=");
  //  Serial.println(xpp);

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(digitalRead(x1));
  Serial.print(":");
  Serial.print(digitalRead(x2));
  Serial.print(":");
  Serial.print(digitalRead(x3));
  Serial.print(":");
  Serial.println(digitalRead(x4));

  delay(200);

}
