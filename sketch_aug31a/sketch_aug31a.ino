 char *name1 = "GUY";
 char *name2 = "DVIR";

 char *names[] = {&name1};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(names[0]);
  Serial.println(names[1]);



}

void loop() {
  // put your main code here, to run repeatedly:

}
