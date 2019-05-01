#ifndef myJSON_h
#define myJSON_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "FS.h"

#define LOG_LENGTH 4
#define DOC_SIZE 512

// template <class T>
class myJSON
{
private:
bool _useSerial=false;
char _filename[30];

public:
char *ver="myJSON_v1.1";
myJSON(char *filename, bool useserial=false);

bool file_exists();
bool file_remove();
bool format ();
bool FS_ok();

void saveJSON2file(JsonDocument& _doc);
void readJSON_file(JsonDocument& _doc);

void printJSON(JsonDocument& _doc);
void PrettyprintJSON(JsonDocument& _doc);

const char *getValue (const char *key);
void removeValue(const char *key);
void eraseArray(char* array_key);
void setValue(const char *key, char *value);
void setValue(const char *key, int  value);
void updateArray(char* array_key, int val);
void printFile();
// void setValue(const char *key, T value);

};
#endif
