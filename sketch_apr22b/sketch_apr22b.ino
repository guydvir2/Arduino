#include <myJSON.h>
#define file "myfile.json"
#define LOG_LENGTH 4

myJSON json(file, true);


// template <typename T>
// void setValue(const char *key, T value){
//         StaticJsonDocument<512> tempJDOC;
//         json.readJSON_file(tempJDOC);
//         tempJDOC[key]=value;
//         json.saveJSON2file(tempJDOC);
//         json.PrettyprintJSON(tempJDOC);
// }


void setup() {
        Serial.println("\nstartUP");
        // Serial.println(json.getValue("Name"));
        // json.format();
        // json.setValue("temp",1);
        // json.setValue("Name","Guy");
        json.updateArray("data",123);
        json.updateArray("data","NEW");
        // json.eraseArray("data");
        // json.removeValue("data");
        json.printFile();


        // char a[100];
        // a=getValue("temp");
        // Serial.println(getValue("temp"));
        // setValue("T",(char)1234);

        // updateArray("data_1", random(1, 50));
        // updateArray("data", "test");

        //  Serial.println("");
        //  json.saveJSON2file(doc);
        //  json.readJSON_file(doc);
        //  Serial.println("After:");
        //  json.PrettyprintJSON(doc);

}

void loop() {
        // put your main code here, to run repeatedly:

}
