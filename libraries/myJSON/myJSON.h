#ifndef myJSON_h
#define myJSON_h

#include "Arduino.h"
// #include <ArduinoJson.h>
// #include "FS.h"

class myJSON
{
public:
// const char *ver = "json_0.1";
myJSON();
void ReadValue();
void saveFile();

private:
int x=0;

};
#endif
