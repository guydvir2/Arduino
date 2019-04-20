#include <myJSON.h>
#define FILENAME "guyfile.txt"
myJSON JSONfile(FILENAME,true);
char b[20];
char json[] =
      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";


void setup(){
  Serial.begin(9600);
  Serial.println("Start");
        // Serial.print("Format: ");
        // Serial.println(JSONfile.format());
        //
        JSONfile.SaveVal("name1","guy");
        JSONfile.getJSON();
        // JSONfile.SaveVal("name2","guy");

        // JSONfile.ReadVal("gps",b);
        // Serial.println(b);
        //
        // Serial.print("file exists: ");
        // Serial.println(JSONfile.exists(FILENAME));
        //
        // Serial.print("Remove file : ");
        // Serial.println(JSONfile.remove(FILENAME));
        // Serial.println(JSONfile.parser(json, "time"));
        // JSONfile.ReadVal("name2",b);
        // Serial.println(b);
        // JSONfile.ReadVal("name2",b);
        // Serial.println(b);


}

void loop() {

}
