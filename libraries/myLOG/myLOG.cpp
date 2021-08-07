#include <myLOG.h>

flashLOG::flashLOG(char *filename)
{
    _logfilename = filename;
}
bool flashLOG::start(int max_entries, int max_entry_len, bool delyedSave, bool debugmode)
{
#if isESP32
    bool a = SPIFFS.begin(true);
#elif isESP8266
    bool a = SPIFFS.begin();
#endif

    if (!a)
    {
        if (_useDebug)
        {
            Serial.println("SPIFFS mount failed");
        }
    }
    _logSize = max_entries;
    _logLength = max_entry_len + 3;
    _useDelayedSave = delyedSave;
    _useDebug = debugmode;

    return a;
}
void flashLOG::looper(int savePeriod)
{
    bool timeCondition = lastUpdate > 0 && millis() - lastUpdate > savePeriod * 1000UL;
    bool overSize_Condition = _buff_i > (int)(0.7 * TEMP_LOG_SIZE);
    if (timeCondition || overSize_Condition)
    {
        _write2file();
        _printDebug("loop_save");
    }
}
void flashLOG::write(const char *message, bool NOW)
{
    char b[_logLength];
    // strncpy(b, message, _logLength - 3);
    // strcpy()
    // sprintf(_logBuffer[_buff_i], "%s%c", b, _EOL);
    sprintf(_logBuffer[_buff_i], "%s%c", message, _EOL);

    Serial.print("add buffer #");
    Serial.print(_buff_i);
    Serial.print(": ");
    Serial.println(_logBuffer[_buff_i]);
    _buff_i++;
    lastUpdate = millis();

    if (!_useDelayedSave || NOW == true)
    {
        _write2file();
        _printDebug("immediate save");
    }
}
void flashLOG::writeNow()
{
    _write2file();
    _printDebug("writeNOW");
}
bool flashLOG::del_line(int line_index)
{
    int c = 0;
    int row_counter = 0;
    char a[_logLength];
    char *tfile = "/tempfile.txt";
    bool line_deleted = false;

    File file1 = SPIFFS.open(_logfilename, "r");
    File file2 = SPIFFS.open(tfile, "w");

    if (!file1 && !file2)
    {
        if (_useDebug)
        {
            Serial.println("Failed to open file for appending");
        }
    }
    else
    {
        while (file1.available())
        {
            char tt = file1.read();
            if (row_counter != line_index) /* copying non-deleted lines */
            {
                if (tt != _EOL)
                {
                    a[c] = tt;
                    c++;
                }
                else
                {
                    a[c] = _EOL;
                    a[c + 1] = '\0';
                    file2.print(a);
                    sprintf(a, "");
                    row_counter++;
                    c = 0;
                }
            }
            else /* ignoring line to be delted */
            {
                if (tt == _EOL)
                {
                    row_counter++;
                    line_deleted = true;
                }
            }
        }
    }
    file1.close();
    file2.close();
    SPIFFS.remove(_logfilename);
    SPIFFS.rename(tfile, _logfilename);
    return line_deleted;
}
bool flashLOG::del_last_record()
{
    int i = getnumlines();
    return del_line(i - 1);
}
void flashLOG::delog()
{
    SPIFFS.remove(_logfilename);
}
bool flashLOG::readline(int r, char retLog[])
{
    int c = 0;
    int row_counter = 0;

    File file = SPIFFS.open(_logfilename, "r");
    if (!file)
    {
        if (_useDebug)
        {
            Serial.println("Failed to open file for reading");
        }
        return 0;
    }
    while (file.available())
    {
        char tt = file.read();
        if (tt == _EOL && row_counter != r)
        {
            row_counter++;
        }
        else if (row_counter == r && tt != _EOL)
        {
            retLog[c] = tt;
            c++;
        }
        else if (row_counter == r && tt == _EOL)
        {
            retLog[c] = _EOL;
            retLog[c + 1] = '\0';
            row_counter++;
        }
    }
    if (row_counter < r)
    {
        // sprintf(retLog, "error-line does not exist");
    }
    file.close();
    return 1;
}
int flashLOG::getnumlines()
{
    int row_counter = 0;

    File file = SPIFFS.open(_logfilename, "r");
    while (file.available())
    {
        char tt = file.read();
        if (tt == _EOL)
        {
            row_counter++;
        }
    }
    file.close();
    return row_counter;
}
int flashLOG::sizelog()
{
    File file = SPIFFS.open(_logfilename, "r");
    int f = file.size();
    file.close();
    return f;
}
void flashLOG::rawPrintfile()
{
    int row_counter = 0;
    bool new_line = true;

    File file = SPIFFS.open(_logfilename, "r");
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
        if (new_line)
        {
            Serial.print("row #");
            Serial.print(row_counter++);
            Serial.print(" {");
            new_line = false;
        }
        char tt = file.read();
        if (tt == _EOL)
        {
            new_line = true;
            Serial.println("}");
        }
        else
        {
            Serial.print(tt);
        }
    }
    file.close();
    Serial.println("~~~ EOF ~~~");
}
void flashLOG::_write2file()
{
    int num_lines = getnumlines();
    if (num_lines > 0 && num_lines + _buff_i > _logSize)
    {
        // _del_lines(num_lines + _buff_i - _logSize);
    }
    File file1 = SPIFFS.open(_logfilename, "a");
    if (!file1)
    {
        if (_useDebug)
        {
            Serial.println("Failed to open file for appending");
        }
    }
    else
    {
        for (int x = 0; x <= _buff_i; x++)
        {
            file1.print(_logBuffer[x]);
            sprintf(_logBuffer[x], "");
        }
        _buff_i = 0;
        lastUpdate = 0;
    }
    _printDebug(" -Saved");
    file1.close();
}
void flashLOG::_del_lines(int line_index)
{
    int c = 0;
    int row_counter = 0;
    char a[_logLength];
    char *tfile = "/tempfile.txt";

    File file1 = SPIFFS.open(_logfilename, "r");
    File file2 = SPIFFS.open(tfile, "w");

    if (!file1 && !file2)
    {
        if (_useDebug)
        {
            Serial.println("Failed to open file for appending");
        }
    }
    else
    {
        while (file1.available())
        {
            char tt = file1.read();
            if (row_counter >= line_index) /* copying non-deleted lines */
            {
            //     if (tt != _EOL)
            //     {
            //         a[c] = tt;
            //         c++;
            //     }
            //     else
            //     {
            //         a[c] = _EOL;
            //         a[c + 1] = '\0';
            //         file2.print(a);
            //         sprintf(a, "");
            //         row_counter++;
            //         c = 0;
            //     }
            }
            else /* ignoring line to be delted */
            {
                if (tt == _EOL)
                {
                    row_counter++;
                    Serial.println(row_counter);
                }
            }
        }
    }
    file1.close();
    file2.close();
    // SPIFFS.remove(_logfilename);
    // SPIFFS.rename(tfile, _logfilename);
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
void flashLOG::printraw()
{
    File file = SPIFFS.open(_logfilename, "r");
    while (file.available())
    {
        char tt = file.read();
        Serial.print(tt);
    }
    file.close();
}