#include <myJSON.h>
#define file "myfile.json"

myJSON json(file, true);
StaticJsonDocument<200> local_doc;

void setup() {
  
  json.readJSON_file(local_doc);
//  local_doc["name1"] = "anna";
//  
  JsonArray data = local_doc.createNestedArray("data");
//  data.add(48.756080);
//  data.add(2.302038);
data.add(12);
Serial.println(data.size());
//  
  json.saveJSON2file(local_doc);
  json.PrettyprintJSON(local_doc);
}

void loop() {
  // put your main code here, to run repeatedly:

}
