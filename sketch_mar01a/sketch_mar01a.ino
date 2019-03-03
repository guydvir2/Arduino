#define PARAM_AMOUNT 4
#define COLOR 1
#define LED_DELAY 20
#define BRIGHTNESS  50
#define LED_DIRECTION 0

char *color_names[] = {"Off", "White", "Red", "Green", "Blue", "Gold", "NavajoWhite"};
char parameters[PARAM_AMOUNT][4];
int param_def[] = {COLOR, BRIGHTNESS, LED_DELAY, LED_DIRECTION};


void setup() {
  Serial.begin(9600);
  Serial.println("Running string 1,2");
  splitter("1,2");
  print_output();

  Serial.println("Running string 1,2,3,4");
  splitter("1,2,3,4");
  print_output();
}


void splitter(char *inputstr) {
  char *pch;
  int i = 0;

  pch = strtok (inputstr, " ,.-");
  while (pch != NULL)
  {
    sprintf(parameters[i],"%s",pch);
    pch = strtok (NULL, " ,.-");
    Serial.println(parameters[i]);
    i++;
  }

  //  update default values
  for (int n = i ; n <PARAM_AMOUNT; n++) {
    sprintf(parameters[n], "%d", param_def[n]);
    Serial.print("param_");
    Serial.print(n);
    Serial.print(" was set to default value:");
    Serial.println(parameters[n]);
    delay(50);
  }
}

void print_output() {
  Serial.println("output:");
  for (int n = 0 ; n < PARAM_AMOUNT; n++) {
    Serial.println(parameters[n]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
