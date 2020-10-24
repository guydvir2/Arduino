#ifndef myJSON_h
#define myJSON_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

class myJSON
{
    #define _def_dsize 400

private:
    bool _useSerial = false;
    bool _openOK = false;
    char _filename[30];

public:
    char *ver = "myJSON_v1.91";
    int DOC_SIZE;
    myJSON(char *filename, bool useserial = false, int doc_size = _def_dsize);

    bool readJSON_file(JsonDocument &_doc);
    bool file_exists();
    bool file_remove();
    bool format();
    bool FS_ok();

    char *retAllJSON();

    bool getValue(const char *key, char *value);
    bool getValue(const char *key, int &retval);
    bool getValue(const char *key, long &retval);
    bool getValue(const char *key, bool &retval);

    void setValue(const char *key, char *value);
    void setValue(const char *key, int value);
    void setValue(const char *key, long value);
    void setValue(const char *key, bool value);

    void add2Array(char *array_key, char *val);
    void add2Array(char *array_key, int val);
    void add2Array(char *array_key, long val);

    void setArrayVal(char *array_key, int i, char *val);
    void setArrayVal(char *array_key, int i, long val);
    void setArrayVal(char *array_key, int i, int val);

    bool getArrayVal(char *array_key, int i, int &retval);
    bool getArrayVal(char *array_key, int i, long &retval);
    bool getArrayVal(char *array_key, int i, char value[20] = "0");

    void nestedArray(char *array_key, long val);
    void eraseArray(char *array_key);

    void printFile();
    void removeValue(const char *key);

private:
    void saveJSON2file(JsonDocument &_doc);

    void printJSON(JsonDocument &_doc);
    void PrettyprintJSON(JsonDocument &_doc);
};


#endif
