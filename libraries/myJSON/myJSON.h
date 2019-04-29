#ifndef myJSON_h
#define myJSON_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "FS.h"


class myJSON
{
private:
bool _useSerial=false;
char _filename[30];

public:
char *ver="myJSON_v1.0";
myJSON(char *filename, bool useserial=false);

bool file_exists();
bool file_remove();
bool format ();
bool FS_ok();

void saveJSON2file(JsonDocument& _doc);
void readJSON_file(JsonDocument& _doc);

void printJSON(JsonDocument& _doc);
void PrettyprintJSON(JsonDocument& _doc);

const char *getValue (char *key);

};
#endif
