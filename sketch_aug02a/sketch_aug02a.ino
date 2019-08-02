#include "myJSON.h"
myJSON json("file.json", true);
void setup() {
  // put your setup code here, to run once:
//  json.format();
  json.updateArray("key1",11);
//  json.updateArray("key2",13);
//  json.updateArray("key3","guyDvir");
//  json.nestedArray("key4",11);
  json.printFile();

}

void loop() {
  // put your main code here, to run repeatedly:

}
