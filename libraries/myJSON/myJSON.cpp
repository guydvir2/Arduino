#include "Arduino.h"
#include "myJSON.h"

myJSON::myJSON(const char *filename, bool useserial, int doc_size)
{
        _useSerial = useserial;
        DOC_SIZE = doc_size;
        sprintf(_filename, "%s", filename);
}
void myJSON::start()
{
#if isESP32
        bool a = LITFS.begin(true);
#elif isESP8266
        bool a = LITFS.begin();
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
        return LITFS.exists(_filename);
}
bool myJSON::file_remove()
{
        return LITFS.remove(_filename);
}
bool myJSON::format()
{
        if (_useSerial)
        {
                Serial.print("Formating...");
        }
        bool flag = LITFS.format();
        return flag;
}
bool myJSON::FS_ok()
{
        return _openOK;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ JSON Functions ~~~~~~~~~~~
void myJSON::_saveJSON2file(JsonDocument &_doc)
{
        File writeFile = LITFS.open(_filename, "w");
        if (writeFile)
        {
                serializeJson(_doc, writeFile);
        }
        writeFile.close();
}
bool myJSON::readJSON_file(JsonDocument &_doc)
{
        File readFile = LITFS.open(_filename, "r");

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
void myJSON::retAllJSON(char *value)
{
        DynamicJsonDocument _tempJDOC(DOC_SIZE);
        readJSON_file(_tempJDOC);
        serializeJson(_tempJDOC, value, measureJson(_tempJDOC) + 1);
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
