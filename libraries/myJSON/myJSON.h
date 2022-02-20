#ifndef myJSON_h
#define myJSON_h
#include <ArduinoJson.h>
#include <FS.h>

#if defined(ESP32)
#include <LITTLEFS.h>
#define isESP32 true
#define isESP8266 false
#elif defined(ARDUINO_ARCH_ESP8266)
#include <LittleFS.h>
#define isESP32 false
#define isESP8266 true
#endif

class myJSON
{
#define _def_dsize 400

private:
    bool _useSerial = false;
    bool _openOK = false;
    char _filename[15];

public:
    const char *ver = "myJSON_v1.95";
    int DOC_SIZE;
    myJSON(const char *filename = "/jfile.json", bool useserial = false, int doc_size = _def_dsize);

    void start();
    bool FS_ok();
    bool format();
    bool file_exists();
    bool file_remove();

    void retAllJSON(char value[]);
    bool readJSON_file(JsonDocument &_doc);
    bool replaceValue(const char *key, char *value);

    bool getValue(const char *key, char *value);
    bool getValue(const char *key, int &retval);
    bool getValue(const char *key, long &retval);
    bool getValue(const char *key, bool &retval);

    void setValue(const char *key, char *value);
    void setValue(const char *key, int value);
    void setValue(const char *key, long value);
    void setValue(const char *key, bool value);
    
    void printFile();
    void removeValue(const char *key);

private:
    void _saveJSON2file(JsonDocument &_doc);
    void _PrettyprintJSON(JsonDocument &_doc);
};

#endif
