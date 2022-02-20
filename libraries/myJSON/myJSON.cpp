#include "Arduino.h"
#include "myJSON.h"

#define LOG_LENGTH 4

myJSON::myJSON(const char *filename, bool useserial, int doc_size)
{
        _useSerial = useserial;
        DOC_SIZE = doc_size;
        sprintf(_filename, "%s", filename);
}
void myJSON::start()
{
#if isESP32
        bool a = LITTLEFS.begin(true);
#elif isESP8266
        bool a = LittleFS.begin();
#endif
        if (a)
        {
                if (_useSerial)
                {
                        Serial.println("file system mount OK");
                        Serial.flush();
                }
                _openOK = true;
        }
        else
        {
                if (_useSerial)
                {
                        Serial.println("Failed to mount file system");
                        Serial.flush();
                }
        }
}

// ~~~~~~~~~~~~~~ File Functions ~~~~~~~~~~~
bool myJSON::file_exists()
{
#if isESP32
        return LITTLEFS.exists(_filename);
#elif isESP8266
        return LittleFS.exists(_filename);
#endif
}
bool myJSON::file_remove()
{
#if isESP8266
        if (LittleFS.begin())
        {
                return LittleFS.remove(_filename);
        }
        else
        {
                return 0;
        }
#elif isESP32
        if (LITTLEFS.begin())
        {
                return LITTLEFS.remove(_filename);
        }
        else
        {
                return 0;
        }
#endif
}
bool myJSON::format()
{
#if isESP32
        bool a = LITTLEFS.begin(true);
#elif isESP8266
        bool a = LittleFS.begin();
#endif

        if (a)
        {
                if (_useSerial)
                {
                        Serial.print("Formating...");
                }
#if isESP32
                bool flag = LITTLEFS.format();
#elif isESP8266
                bool flag = LittleFS.format();
#endif
                if (_useSerial)
                {
                        if (flag)
                        {
                                // Serial.println("Done");
                        }
                        else
                        {
                                // Serial.println("Failed");
                        }
                }
                return flag;
        }
        else
                return 0;
}
bool myJSON::FS_ok()
{
        return _openOK;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ JSON Functions ~~~~~~~~~~~
void myJSON::_saveJSON2file(JsonDocument &_doc)
{
        #if isESP8266
        File writeFile = LittleFS.open(_filename, "w");
        #elif isESP32
        File writeFile = LITTLEFS.open(_filename, "w");
        #endif
        serializeJson(_doc, writeFile);
        writeFile.close();
        // myJSON::_PrettyprintJSON(_doc);
}
bool myJSON::readJSON_file(JsonDocument &_doc)
{
        #if isESP8266
        File readFile = LittleFS.open(_filename, "r");
        #elif isESP32
        File readFile = LITTLEFS.open(_filename, "r");
        #endif

        DeserializationError error = deserializeJson(_doc, readFile);
        if (error)
        {
                if (_useSerial)
                {
                        Serial.println(F("Failed to read JSON file"));
                        Serial.println(_filename);
                        Serial.println(error.c_str());
                }
                readFile.close();
                return 0;
        }
        readFile.close();
        return 1;
}
void myJSON::_PrettyprintJSON(JsonDocument &_doc)
{
        serializeJsonPretty(_doc, Serial);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~ User Functions : JSON + file saving ~~~~~~~~~~~

bool myJSON::getValue(const char *key, char *value)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        bool hasKey = _tempJDOC.containsKey(key);
        if (hasKey)
        {
                const char *val = _tempJDOC[key];
                sprintf(value, "%s", val);
                return 1;
        }
        else
        {
                return 0; // when key is not present
        }
}
bool myJSON::getValue(const char *key, int &retval)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        bool hasKey = _tempJDOC.containsKey(key);
        if (hasKey)
        {
                retval = _tempJDOC[key];
                return 1;
        }
        else
        {
                return 0; // when key is not present
        }
}
bool myJSON::getValue(const char *key, long &retval)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        bool hasKey = _tempJDOC.containsKey(key);
        if (hasKey)
        {
                retval = _tempJDOC[key];
                return 1;
        }
        else
        {
                // Serial.println("NOT SUCH KEY");
                return 0; // when key is not present
        }
}
bool myJSON::getValue(const char *key, bool &retval)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        bool hasKey = _tempJDOC.containsKey(key);
        if (hasKey)
        {
                retval = _tempJDOC[key];
                return 1;
        }
        else
        {
                return 0; // when key is not present
        }
}

void myJSON::setValue(const char *key, char *value)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        _tempJDOC[key] = value;
        _saveJSON2file(_tempJDOC);
}
void myJSON::setValue(const char *key, int value)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        _tempJDOC[key] = value;
        _saveJSON2file(_tempJDOC);
}
void myJSON::setValue(const char *key, long value)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        _tempJDOC[key] = value;
        _saveJSON2file(_tempJDOC);
}
void myJSON::setValue(const char *key, bool value)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        _tempJDOC[key] = value;
        _saveJSON2file(_tempJDOC);
}
void myJSON::removeValue(const char *key)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        _tempJDOC.remove(key);
        _saveJSON2file(_tempJDOC);
}
void myJSON::retAllJSON(char value[])
{
        char value2[500];
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        serializeJson(_tempJDOC, value2);
        strcpy(value, value2);
}
void myJSON::printFile()
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        if (file_exists())
        {
                readJSON_file(_tempJDOC);
                Serial.println(_filename);
                serializeJsonPretty(_tempJDOC, Serial);
                Serial.println();
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
