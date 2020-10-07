#include <myLOG.h>

flashLOG::flashLOG(char *filename)
{
    _logfilename = filename;
}
void flashLOG::start(int max_entries, int max_entry_len, bool delyedSave)
{
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS mount failed");
    }
    _logsize = max_entries;
    _log_length = max_entry_len;
    _useDelayedSave = delyedSave;
}
void flashLOG::looper(int savePeriod)
{
    if (lastUpdate > 0 && millis() - lastUpdate > savePeriod * 1000L || _buff_i > (int)0.7 * LOG_SIZE)
    {
        _write2file();
    }
}
void flashLOG::write(const char *message)
{
    char b[_log_length + 3];
    strncpy(b, message, _log_length);
    sprintf(_logBuffer[_buff_i], "%s%c", b, _EOL);
    if(_useDelayedSave){
    lastUpdate = millis();
    _buff_i++;
    }
    else{
        _write2file();
    }
}
void flashLOG::_write2file()
{
    int num_lines = getnumlines();
    if (num_lines > 0 && num_lines + _buff_i > _logsize)
    {
        // Serial.printf("delteing %d lines \n", (num_lines + _buff_i) - _logsize);
        _del_lines((num_lines + _buff_i) - _logsize);
    }

    File file1 = SPIFFS.open(_logfilename, "a");
    if (!file1)
    {
        Serial.println("Failed to open file for appending");
    }
    else
    {
        for (int x = 0; x < _buff_i; x++)
        {
            file1.print(_logBuffer[x]);
        }
    }
    file1.close();
    _buff_i = 0;
    lastUpdate = 0;
}
void flashLOG::_del_lines(byte line_index)
{
    int c = 0;
    int row_counter = 0;
    char a[_log_length + 3];
    char *tfile = "/tempfile.txt";

    File file1 = SPIFFS.open(_logfilename, "r");
    File file2 = SPIFFS.open(tfile, "w");

    if (!file1 && !file2)
    {
        Serial.println("Failed to open file for appending");
    }
    else
    {
        while (file1.available())
        {
            char tt = file1.read();
            if (row_counter >= line_index)
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
            else
            {
                if (tt == _EOL)
                {
                    row_counter++;
                }
            }
        }

        file1.close();
        file2.close();
        SPIFFS.remove(_logfilename);
        SPIFFS.rename(tfile, _logfilename);
    }
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
        Serial.println("Failed to open file for reading");
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