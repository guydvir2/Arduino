#include <ArduinoJson.h>
// compute the required size
//const size_t CAPACITY = JSON_ARRAY_SIZE(3);
//const int CAPACITY = 250;

// allocate the memory for the document
StaticJsonDocument<1024> doc;

// create an empty array
JsonArray array = doc.to<JsonArray>();

// add some values
array.add("hello");
array.add(42);
array.add(3.14);

// serialize the array and send the result to Serial
serializeJson(doc, Serial);

void setup(){
}

void loop(){
}
}
