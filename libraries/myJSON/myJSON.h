#ifndef myJSON_h
#define myJSON_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "FS.h"

class myJSON
{
public:

myJSON(char *filename, bool useserial=false);
bool ReadVal(char* key, char *ret_value);
bool SaveVal(char *key, char *value);
bool parser(char *json_data);
bool exists(char *path);
bool remove(char *path);
bool format ();

private:
bool useSerial;
char _filename[30];

};
#endif
