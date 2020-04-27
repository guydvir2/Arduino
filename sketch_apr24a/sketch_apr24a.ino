const int num_sw = 1;
int pins[] = {3,2};
bool laststate[num_sw];
bool first_det[num_sw];
void setup() {
  // put your setup code here, to run once:
  pinMode(pins[0], INPUT);
  pinMode(pins[1], INPUT);

  Serial.begin(9600);
  Serial.println("Start");
}

void loop() {
  // put your main code here, to run repeatedly:
  bool reads[num_sw];
  for (int i = 0; i < num_sw; i++) {
    reads[i] = digitalRead(pins[i]);
    if (reads[i] != laststate[i]) {
      char t[50];
      if (reads[i] == HIGH && first_det[0] == false ){
        
      }
      laststate[i] = reads[i];
      sprintf(t,"sensor [#%d] is [%s]",i,reads[i]? "HIGH":"LOW");
      Serial.println(t);
    }
  }
  delay(500);
  
}
