//bool isESP8266=false;
bool isESP32=false;
#if defined (ARDUINO_ARCH_ESP8266)
// do one thing
#define BAORDTYPE "ESP8266"

#elif defined(ESP32)
// do another
#define BAORDTYPE "ESP32"
#else
#error Architecture unrecognized by this code.
#endif

void setup(){
  Serial.begin(9600);
  Serial.println(BAORDTYPE);
}

void loop(){
  
}
