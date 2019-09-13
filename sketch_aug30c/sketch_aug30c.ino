#include <myJSON.h>
#include <ArduinoJson.h>
#include "FS.h"

myJSON testInflash("file0.json", true);


void setup() {
        Serial.begin(9600);
        testInflash.format();
        // testInflash.setValue("val0",13);
        // testInflash.updateArray("array_key",1234);
        // int a;
        // for(int a=0; a<5; a++) {
        //         testInflash.setArrayVal("array_key",2,5);
        // }
        // testInflash.add2Array("array_key", 5);
        // testInflash.setArrayVal("array_key3",0,13359);
        // testInflash.setArrayVal("array_key2",11,12313);
        // Serial.print("a=");
        // Serial.println(a);
        // StaticJsonDocument<256> doc;
        // doc["flag"]="on";
        //
        // JsonArray data = doc.createNestedArray("data");
        // Serial.println(data.size());
        // // data.add(48.756080);
        // // data.add(2.302038);
        // // data[0]=12345;
        // serializeJson(doc,Serial);
        // JsonArray array = doc.to<JsonArray>();
        // array.add(454);
        //
        // serializeJsonPretty(doc,Serial);


        // testInflash.setValue("Guy1",22000);
        // testInflash.setArrayVal("val0",1,100);
        //
        testInflash.printFile();
//


}

void loop() {

}
