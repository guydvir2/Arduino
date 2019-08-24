#include "myJSON.h"
myJSON json("file0.json", true);
char *clock_fields[] = {"ontime", "off_time", "flag"};



void setup() {
        // put your setup code here, to run once:
        //json.nestedArray("key1",1024);
        //json.nestedArray("key2",2048);
//  json.format();
//  for (int n = 0; n < 2; n++) {
//    for (int i = 0; i < 3; i++) {
//      char temp[10];
//      sprintf(temp, "%s_%d", clock_fields[i], n);
//      json.updateArray(temp, n);
//
//    }
//
//  }
//json.eraseArray(clock_fields[0]);
        json.updateArray(clock_fields[0], "Hello");
        json.updateArray(clock_fields[0], "Guy");
        json.updateArray(clock_fields[0], "dvir");

        json.updateArray(clock_fields[1], 17);
        json.updateArray(clock_fields[1], 0);
        json.updateArray(clock_fields[1], 0);

        json.updateArray(clock_fields[2], 1);

        int a;
        char temp[20];
//json.getArrayVal(clock_fields[0],0,temp);
        json.getArrayVal(clock_fields[1],1,a);

        Serial.print("myVal:");
        Serial.println(a);

        json.printFile();
}

void loop() {
        // put your main code here, to run repeatedly:

}
