void setup() {
  Serial.begin(9600);
  delay(50);
  Serial.println("");
  Serial.println("HI");

  char tempstr[120];
  char states[120];
  int NUM_SWITCHES = 4;


  strcpy(states, "dvir");
    for (int i = 0; i < NUM_SWITCHES; i++) {
      sprintf(tempstr, "guyDvir [%d]", i);
      strcat(states, tempstr);
    }
  Serial.println(states);
}

void loop() {
  // put your main code here, to run repeatedly:

}
