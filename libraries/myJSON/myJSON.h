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
const char *parser(char *json_data, const char *key);
bool exists(char *path);
bool remove(char *path);
bool format ();
char getJSON();

private:
bool useSerial;
char _filename[30];

};
#endif
