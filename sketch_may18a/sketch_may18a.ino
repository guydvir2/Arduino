#include <myJSON.h>
#define file "myfile.json"
#define LOG_LENGTH 4

myJSON json(file, true);

void setup() {
  // put your setup code here, to run once:
//  json.setValue("2019","Jan");
json.updateArray("vvv","Jan");
json.updateArray("vvv","Feb");

  
  json.printFile();

}

void loop() {
  // put your main code here, to run repeatedly:

}
