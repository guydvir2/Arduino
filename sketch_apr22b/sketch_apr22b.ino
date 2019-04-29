#include <myJSON.h>
#define file "myfile.json"
#define LOG_LENGTH 4

myJSON json(file, true);

template <typename T1>
void updateArray(char* array_key, T1 val) {
        StaticJsonDocument<512> tempJDOC;
        json.readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull()) {
                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH) {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH) {
                for (int n = 0; n < LOG_LENGTH - 1; n++) {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        json.saveJSON2file(tempJDOC);
        json.PrettyprintJSON(tempJDOC);
}

template <typename T>
void setValue(const char *key, T value){
        StaticJsonDocument<512> tempJDOC;
        json.readJSON_file(tempJDOC);
        tempJDOC[key]=value;
        json.saveJSON2file(tempJDOC);
        json.PrettyprintJSON(tempJDOC);
}

// const char *getValue (char *key){
//         StaticJsonDocument<512> tempJDOC;
//         json.readJSON_file(tempJDOC);
//         const char *a = tempJDOC[key];
//         // char ret_val[100];
//         // strcpy(ret_val, a);
//         return a;
// }

void setup() {
        Serial.println("");
        json.format();
        // setValue("temp",1);
        // setValue("Name","Guy");

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
