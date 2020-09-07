#include "Arduino.h"
#include "myJSON.h"
#include "FS.h"
#include <ArduinoJson.h>

#define LOG_LENGTH 4

myJSON::myJSON(char *filename, bool useserial, int doc_size)
{
        _useSerial = useserial;
        DOC_SIZE = doc_size;
        sprintf(_filename, "%s", filename);
        if (_useSerial)
        {
                Serial.begin(9600);
        }
        if (!SPIFFS.begin())
        {
                if (_useSerial)
                {
                        Serial.println("Failed to mount file system");
                }
        }
        else
        {
                if (SPIFFS.exists(_filename))
                {
                        _openOK = true;
                }
        }
}

// ~~~~~~~~~~~~~~ File Functions ~~~~~~~~~~~
bool myJSON::file_exists()
{
        if (SPIFFS.begin())
        {
                return SPIFFS.exists(_filename);
        }
}
bool myJSON::file_remove()
{
        if (SPIFFS.begin())
        {
                return SPIFFS.remove(_filename);
        }
}
bool myJSON::format()
{
        if (SPIFFS.begin())
        {
                if (_useSerial)
                {
                        Serial.print("Formating...");
                }
                bool flag = SPIFFS.format();
                if (_useSerial)
                {
                        if (flag)
                        {
                                Serial.println("Done");
                        }
                        else
                        {
                                Serial.println("Failed");
                        }
                }
                return flag;
        }
}
bool myJSON::FS_ok()
{
        return _openOK;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~ JSON Functions ~~~~~~~~~~~
void myJSON::saveJSON2file(JsonDocument &_doc)
{
        File writeFile = SPIFFS.open(_filename, "w");
        serializeJson(_doc, writeFile);
        delay(50);
        // Serial.println("JSON file saved OK");
        // myJSON::PrettyprintJSON(_doc);
}
bool myJSON::readJSON_file(JsonDocument &_doc)
{
        File readFile = SPIFFS.open(_filename, "r");
        DeserializationError error = deserializeJson(_doc, readFile);
        if (error)
        {
                Serial.println(F("Failed to read JSON file"));
                Serial.println(error.c_str());
                readFile.close();
                return 0;
        }
        else
        {
                serializeJson(_doc, readFile);
                readFile.close();
                return 1;
        }
        delay(50);
}
void myJSON::printJSON(JsonDocument &_doc)
{
        serializeJson(_doc, Serial);
}
void myJSON::PrettyprintJSON(JsonDocument &_doc)
{
        serializeJsonPretty(_doc, Serial);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~ User Functions : JSON + file saving ~~~~~~~~~~~
bool myJSON::getValue(const char *key, char *value)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey)
        {
                const char *val = tempJDOC[key];
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
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey)
        {
                retval = tempJDOC[key];
                return 1;
        }
        else
        {
                return 0; // when key is not present
        }
}
bool myJSON::getValue(const char *key, long &retval)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey)
        {
                retval = tempJDOC[key];
                return 1;
        }
        else
        {
                return 0; // when key is not present
        }
}
bool myJSON::getValue(const char *key, bool &retval)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        bool hasKey = tempJDOC.containsKey(key);
        if (hasKey)
        {
                retval = tempJDOC[key];
                return 1;
        }
        else
        {
                return 0; // when key is not present
        }
}

void myJSON::setValue(const char *key, char *value)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        tempJDOC[key] = value;
        saveJSON2file(tempJDOC);
}
void myJSON::setValue(const char *key, int value)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        tempJDOC[key] = value;
        saveJSON2file(tempJDOC);
}
void myJSON::setValue(const char *key, long value)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        tempJDOC[key] = value;
        saveJSON2file(tempJDOC);
}
void myJSON::setValue(const char *key, bool value)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        tempJDOC[key] = value;
        saveJSON2file(tempJDOC);
}

void myJSON::add2Array(char *array_key, char *val)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull())
        { // create for the first time
                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH)
        {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH)
        {
                for (int n = 0; n < LOG_LENGTH - 1; n++)
                {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        saveJSON2file(tempJDOC);
}
void myJSON::add2Array(char *array_key, int val)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull())
        { // create for the first time
                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH)
        {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH)
        {
                for (int n = 0; n < LOG_LENGTH - 1; n++)
                {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        saveJSON2file(tempJDOC);
}
void myJSON::add2Array(char *array_key, long val)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        if (data_key.isNull())
        { // create for the first time

                JsonArray data = tempJDOC.createNestedArray(array_key);
                data.add(val);
        }
        else if (tempJDOC[array_key].size() < LOG_LENGTH)
        {
                tempJDOC[array_key].add(val);
        }
        else if (tempJDOC[array_key].size() >= LOG_LENGTH)
        {
                for (int n = 0; n < LOG_LENGTH - 1; n++)
                {
                        tempJDOC[array_key][n] = tempJDOC[array_key][n + 1];
                }
                tempJDOC[array_key][LOG_LENGTH - 1] = val;
        }
        saveJSON2file(tempJDOC);
}

bool myJSON::getArrayVal(char *array_key, int i, int &retval)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        bool hasKey = tempJDOC.containsKey(array_key);
        if (hasKey)
        {
                retval = tempJDOC[array_key][i].as<int>();
                return 1;
        }
        else
        {
                return 0;
        }
}
bool myJSON::getArrayVal(char *array_key, int i, long &retval)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        bool hasKey = tempJDOC.containsKey(array_key);
        if (hasKey)
        {
                retval = tempJDOC[array_key][i].as<long>();
                return 1;
        }
        else
        {
                return 0;
        }
}
bool myJSON::getArrayVal(char *array_key, int i, char value[20])
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        bool hasKey = tempJDOC.containsKey(array_key);
        if (hasKey)
        {
                char val[20];
                // strcpy(val,tempJDOC[array_key][i].as<char>());
                // sprintf(value,"%s",val);//tempJDOC[array_key][i].as<char>());
                return 1;
        }
        else
        {
                return 0;
        }
}

void myJSON::setArrayVal(char *array_key, int i, int val)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        if (tempJDOC.containsKey(array_key))
        { // Key is alreay set
                JsonVariant data_key = tempJDOC[array_key];
                // if (data_key.size()==0) {
                //         Serial.println("HAVEKEY_BUT ZERO");
                //         data_key.add(0);
                // }
                if (data_key.size() <= i)
                {
                        for (int a = data_key.size(); a <= i; a++)
                        {
                                data_key.add(0);
                        }
                }
                data_key[i] = val;
        }
        else
        { // Create Key
                JsonArray array_data = tempJDOC.createNestedArray(array_key);
                for (int a = 0; a <= i; a++)
                {
                        array_data.add(0);
                }
                array_data[i] = val;
        }
        saveJSON2file(tempJDOC);
}
void myJSON::setArrayVal(char *array_key, int i, long val)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        if (tempJDOC.containsKey(array_key))
        { // Key is alreay set
                JsonVariant data_key = tempJDOC[array_key];
                // if (data_key.size()==0) {
                //         Serial.println("HAVEKEY_BUT ZERO");
                //         data_key.add(0);
                // }
                if (data_key.size() <= i)
                {
                        for (int a = data_key.size(); a <= i; a++)
                        {
                                data_key.add(0);
                        }
                }
                data_key[i] = val;
        }
        else
        { // Create Key
                JsonArray array_data = tempJDOC.createNestedArray(array_key);
                for (int a = 0; a <= i; a++)
                {
                        array_data.add(0);
                }
                array_data[i] = val;
        }
        saveJSON2file(tempJDOC);
}
void myJSON::setArrayVal(char *array_key, int i, char *val)
{
        // DynamicJsonDocument tempJDOC(DOC_SIZE);
        // myJSON::readJSON_file(tempJDOC);
        //
        // tempJDOC[array_key][i] = val;
        // myJSON::saveJSON2file(tempJDOC);
}

void myJSON::nestedArray(char *array_key, long val)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        JsonArray array = tempJDOC.to<JsonArray>();
        JsonObject nested = array.createNestedObject();
        nested[array_key] = val;
        saveJSON2file(tempJDOC);
}

void myJSON::eraseArray(char *array_key)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);

        JsonVariant data_key = tempJDOC[array_key];
        JsonArray data = tempJDOC.createNestedArray(array_key);
        saveJSON2file(tempJDOC);
}
void myJSON::removeValue(const char *key)
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        readJSON_file(tempJDOC);
        tempJDOC.remove(key);
        saveJSON2file(tempJDOC);
}

void myJSON::printFile()
{
        DynamicJsonDocument tempJDOC(DOC_SIZE);
        if (file_exists())
        {
                readJSON_file(tempJDOC);
                serializeJsonPretty(tempJDOC, Serial);
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
