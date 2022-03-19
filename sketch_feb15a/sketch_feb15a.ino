int outputPin = D1;
int inputPin = D5;

const char *a = "-0.1237676";
const char *b = "1";
const char *c = "guydvir";
const char *d = "true";
const char *e = "0";
const char *f = "A";
const char *g = "-1";
const char *h = "true";

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Start");
  //  pinMode(outputPin, OUTPUT);
  pinMode(inputPin, INPUT_PULLUP);
  //    digitalWrite(outputPin, LOW);
  // analogWrite(outputPin,0);
  // Serial.println(atoi(a));
}

void loop()
{
  //    digitalWrite(outputPin, !digitalRead(outputPin));
  //    Serial.println(digitalRead(outputPin));
  //  for (int i = 0; i < 256; i++) {
  //    analogWrite(outputPin, i);
  //    delay(10);
  //  }
  //    digitalWrite(outputPin, HIGH);
  //    delay(10);
  //    digitalWrite(outputPin, LOW);
  //    delay(1000);
  // Serial.println(digitalRead(inputPin));
  delay(2000);
  const char *y = h;
  Serial.print("\n\nUUT: ");
  Serial.println(y);
  int x = strlen(y);
  Serial.print("len:");
  Serial.println(x);
  // Serial.println(atof(y));//, strlen(y)-2);
  Serial.print("conv to int: ");
  Serial.println(atoi(y));
  Serial.print("conv to float: ");
  Serial.println(atof(y));

  int i = atoi(y);
  float f = atof(y);

  if (isAlpha(y[0]))
  {
    if (strcmp(y, "true") == 0 || strcmp(y, "false") == 0)
    {
      Serial.println("Bool");
    }
    else
    {
      Serial.println("string");
    }
  }
  else
  {
    if (i != f)
    {
      Serial.println("Float");
    }
    else
    {
      Serial.println("int");
    }
  }
}
