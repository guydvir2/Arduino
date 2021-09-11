#include <Arduino.h>
#include "myLOG.h"

flashLOG::flashLOG(char *filename)
{
    _logfilename = filename;
}
bool flashLOG::start(uint8_t max_entries, uint8_t max_entry_len, bool delyedSave, bool debugmode)
{
    _logSize = max_entries;
    _logLength = max_entry_len;
    _useDelayedSave = delyedSave;
    _useDebug = debugmode;

#if isESP32
    bool a = LITTLEFS.begin(true);
#elif isESP8266
    bool a = LittleFS.begin();
#endif

    if (!a)
    {
        if (_useDebug)
        {
            Serial.println("LittleFS mount failed");
        }
    }
    return a;
}
void flashLOG::looper(uint8_t savePeriod)
{
    bool timeCondition = lastUpdate > 0 && millis() - lastUpdate > savePeriod * 1000UL;
    bool overSize_Condition = _buff_i > (int)(0.7 * _logSize);
    if (timeCondition || overSize_Condition)
    {
        _write2file();
        // _printDebug("loop_save");
    }
}
void flashLOG::write(const char *message, bool NOW)
{
    sprintf(_logBuffer[_buff_i], "%s", message);

    _buff_i++;
    lastUpdate = millis();

    if (!_useDelayedSave || NOW == true)
    {
        _write2file();
        // _printDebug("immediate save");
    }
}
void flashLOG::rawPrintfile()
{
    uint8_t row_counter = 0;

#if isESP8266
    File file = LittleFS.open(_logfilename, "r");
#elif isESP32
    File file = LITTLEFS.open(_logfilename, "r");
#endif

    if (!file)
    {
        if (_useDebug)
        {
            Serial.println("Failed to open file for reading");
        }
    }

    Serial.print("~~~ Saved in ");
    Serial.print(_logfilename);
    Serial.println(" ~~~");
    while (file.available())
    {
        String line = file.readStringUntil(_EOL);
        String lineFormat = "row #" + String(row_counter) + " {" + line + "}";
        Serial.println(lineFormat);
        row_counter++;
    }
    Serial.println("~~~ EOF ~~~");
    file.close();
}
bool flashLOG::del_line(uint8_t line_index)
{
    uint8_t row_counter = 0;
    char *tfile = "/tempfile.txt";
    bool line_deleted = false;
#if isESP8266
    File file1 = LittleFS.open(_logfilename, "r");
    File file2 = LittleFS.open(tfile, "w");
#elif isESP32
    File file1 = LITTLEFS.open(_logfilename, "r");
    File file2 = LITTLEFS.open(tfile, "w");
#endif

    if (_chkFileOK(file1) && _chkFileOK(file2))
    {
        while (file1.available())
        {
            String line = file1.readStringUntil(_EOL);
            if (line_index != row_counter)
            {
                file2.println(line);
            }
            row_counter++;
        }
    }

    file1.close();
    file2.close();
#if isESP8266
    LittleFS.remove(_logfilename);
    LittleFS.rename(tfile, _logfilename);
#elif isESP32
    LITTLEFS.remove(_logfilename);
    LITTLEFS.rename(tfile, _logfilename);
#endif
    return line_deleted;
}
bool flashLOG::del_last_record()
{
    uint8_t i = getnumlines();
    return del_line(i - 1);
}
bool flashLOG::readline(uint8_t r, char retLog[])
{
    uint8_t row_counter = 0;
#if isESP8266
    File file = LittleFS.open(_logfilename, "r");
#elif isESP32
    File file = LITTLEFS.open(_logfilename, "r");
#endif

    if (_chkFileOK(file))
    {
        while (file.available() || row_counter != r)
        {
            String line = file.readStringUntil(_EOL);
            if (row_counter == r)
            {
                int len = line.length() + 1;
                line.toCharArray(retLog, len);
                file.close();
                return 1;
            }
            row_counter++;
        }
    }
    return 0;
}
bool flashLOG::delog()
{
#if isESP8266
    return LittleFS.remove(_logfilename);
#elif isESP32
    return LITTLEFS.remove(_logfilename);
#endif
}
uint8_t flashLOG::getnumlines()
{
    uint8_t row_counter = 0;
#if isESP8266
    File file = LittleFS.open(_logfilename, "r");
#elif isESP32
    File file = LITTLEFS.open(_logfilename, "r");
#endif
    if (file)
    {
        while (file.available())
        {
            file.readStringUntil(_EOL);
            row_counter++;
        }
    }
    file.close();
    return row_counter;
}
uint8_t flashLOG::sizelog()
{
#if isESP8266
    File file = LittleFS.open(_logfilename, "r");
#elif isESP32
    File file = LITTLEFS.open(_logfilename, "r");
#endif
    int f = file.size();
    file.close();
    return f;
}

bool flashLOG::_chkFileOK(File &_file)
{
    if (!_file)
    {
        if (_useDebug)
        {
            Serial.println("Failed to open file for appending");
        }
        return 0;
    }
    else
    {
        return 1;
    }
}
bool flashLOG::_write2file()
{
    bool _line_added = false;
    uint8_t num_lines = getnumlines();
    if (_logSize - 1 < num_lines + _buff_i)
    {
        _del_lines(num_lines + _buff_i + 1 - _logSize);
    }
#if isESP8266
    File file1 = LittleFS.open(_logfilename, "a+");
#elif isESP32
    File file1 = LITTLEFS.open(_logfilename, "a+");
#endif

    if (_chkFileOK(file1))
    {
        for (int x = 0; x < _buff_i; x++)
        {
            file1.println(_logBuffer[x]);
            _line_added = true;
        }
        _buff_i = 0;
        lastUpdate = 0;
    }
    file1.close();
    if (_useDebug)
    {
        rawPrintfile();
    }
    return _line_added;
}
bool flashLOG::_del_lines(uint8_t line_index)
{
    uint8_t row_counter = 0;
    char *tfile = "/tempfile.txt";
    bool _delted_lines = false;
#if isESP8266
    File file1 = LittleFS.open(_logfilename, "r");
    File file2 = LittleFS.open(tfile, "w");
#elif isESP32
    File file1 = LITTLEFS.open(_logfilename, "r");
    File file2 = LITTLEFS.open(tfile, "w");
#endif

    if (_chkFileOK(file1) && _chkFileOK(file2))
    {
        while (file1.available())
        {
            String line = file1.readStringUntil(_EOL);
            row_counter++;
            if (row_counter >= line_index) /* copying non-deleted lines */
            {
                file2.println(line);
                _delted_lines = true;
            }
        }
    }
    file1.close();
    file2.close();
#if isESP8266
    LittleFS.remove(_logfilename);
    LittleFS.rename(tfile, _logfilename);
#elif isESP32
    LITTLEFS.remove(_logfilename);
    LITTLEFS.rename(tfile, _logfilename);
#endif
    return _delted_lines;
}
void flashLOG::_printDebug(char *msg)
{
    if (_useDebug)
    {
        Serial.print(_logfilename);
        Serial.print(": ");
        Serial.println(msg);
    }
}