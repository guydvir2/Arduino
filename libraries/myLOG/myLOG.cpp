#include <Arduino.h>
#include "myLOG.h"

flashLOG::flashLOG(char *filename)
{
    _logfilename = filename;
}
bool flashLOG::start(uint8_t max_entries, bool delyedSave, bool debugmode)
{
    _logSize = max_entries;
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
    if (_delayed_save(savePeriod))
    {
        _write2file();
    }
}
void flashLOG::write(const char *message, bool NOW)
{
    _insert_record_to_buffer(message);

    if (NOW == true || !_useDelayedSave)
    {
        _write2file();
    }
    else
    {
        lastUpdate = millis();
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
            else
            {
                line_deleted = true;
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
    file.close();
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
int flashLOG::getnumlines()
{
    int row_counter = 0;
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
unsigned long flashLOG::sizelog()
{
#if isESP8266
    File file = LittleFS.open(_logfilename, "r");
#elif isESP32
    File file = LITTLEFS.open(_logfilename, "r");
#endif
    unsigned long f = file.size();
    file.close();
    return f;
}
bool flashLOG::_delayed_save(uint8_t _savePeriod)
{
    bool timeCondition = lastUpdate > 0 && millis() - lastUpdate > _savePeriod * 1000UL;
    bool overSize_Condition = _logBuff.length() > 800; // About 6 entries
    return timeCondition || overSize_Condition;
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
void flashLOG::_emptyBuffer()
{
    _logBuff = "";
}
void flashLOG::_insert_record_to_buffer(const char *inmsg)
{
    _logBuff += String(inmsg);
    _logBuff += _EOL;
}
int flashLOG::_getBuffer_records()
{
    int _end = 0;
    int _start = 0;
    int lineCounter = 0;

    if (_logBuff.length() > 0)
    {
        while (true)
        {
            _end = _logBuff.indexOf(_EOL, _start);
            if (_end == -1)
            {
                return 0;
            }
            else if (_end == _logBuff.length() - 1)
            {
                return ++lineCounter;
            }
            else
            {
                _start = _end + 1;
                lineCounter++;
            }
        }
    }
    else
    {
        return 0;
    }
}
String flashLOG::_getBuffer_line(int requested_line)
{
    int _start = 0;
    int _end = 0;
    int lineCounter = 0;

    if (_logBuff.length() > 0 && requested_line >= 0)
    {
        while (lineCounter <= requested_line)
        {
            _end = _logBuff.indexOf(_EOL, _start);
            if (lineCounter == requested_line)
            {
                return _logBuff.substring(_start, _end);
            }
            else if (_end > _logBuff.length() - 1)
            {
                return "Error_0";
            }
            else
            {
                _start = _end + 1;
                lineCounter++;
            }
        }
    }
    else
    {
        return "Error_1";
    }
}
bool flashLOG::_write2file()
{
    bool _line_added = false;
    int _m = _getBuffer_records();     // Lines stored in buffer
    int num_lines = getnumlines(); // entries stored
    if (_logSize - 1 < num_lines + _m)
    {
        _del_lines(num_lines + _m + 1 - _logSize);
    }

#if isESP8266
    File file1 = LittleFS.open(_logfilename, "a+");
#elif isESP32
    File file1 = LITTLEFS.open(_logfilename, "a+");
#endif

    if (_chkFileOK(file1))
    {
        for (int x = 0; x < _m; x++)
        {
            file1.println(_getBuffer_line(x));
            _line_added = true;
        }
        _emptyBuffer();
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