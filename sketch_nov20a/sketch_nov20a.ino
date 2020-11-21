#include <myJSON.h>

myJSON jSave("test.json", true);
void setup() {
  // put your setup code here, to run once:
//  Serial.begin(9600);
  Serial.println("OK");
  jSave.start();
//  jSave.format();
  Serial.println(jSave.file_exists());
  jSave.setValue("key13",(int)125);

}

void loop() {
  // put your main code here, to run repeatedly:

}
