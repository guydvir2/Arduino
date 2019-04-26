#include <myJSON.h>
#define file "myfile.json"
#define LOG_LENGTH 4

myJSON json(file, true);

void updateArray(char* array_key, int val) {
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

void setValue(char *key, char* value){
        StaticJsonDocument<512> tempJDOC;
        json.readJSON_file(tempJDOC);
        tempJDOC[key]=value;
        json.saveJSON2file(tempJDOC);
        json.PrettyprintJSON(tempJDOC);
}

void getValue (char *key, char ret_val[100]){
        StaticJsonDocument<512> tempJDOC;
        json.readJSON_file(tempJDOC);
        strcpy(ret_val, tempJDOC[key]);
}

void setup() {
        Serial.println("");
        // json.format();
        // setValue("temp","HIGH");
        char a[20];
        getValue("temp", a);
        Serial.println(a);
        // setValue("T",(char)1234);

        // updateArray("data", random(1, 50));
        // updateArray("data_1", random(1, 50));

        //  Serial.println("");
        //  json.saveJSON2file(doc);
        //  json.readJSON_file(doc);
        //  Serial.println("After:");
        //  json.PrettyprintJSON(doc);

}

void loop() {
        // put your main code here, to run repeatedly:

}
