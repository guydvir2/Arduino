#include <myLOG.h>

flashLOG::flashLOG(char *filename)
{
    _logfilename = filename;
}
void flashLOG::start()
{
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS mount failed");
    }
}
void flashLOG::write(const char *message)
{
    char a[_log_length];
    int num_lines = read();
    if (num_lines > 0)
    {
        for (int a = min(num_lines, _logsize - 1); a > 0; a--)
        {
            sprintf(_log_array[a], _log_array[a - 1]);
        }
    }
    sprintf(a, "%s%c", message, _EOL);
    strcpy(_log_array[0], a);

    File file1 = SPIFFS.open(_logfilename, "w");
    if (!file1)
    {
        Serial.println("Failed to open file for appending");
    }
    else
    {
        for (int x = 0; x <= min(num_lines, _logsize - 1); x++)
        {
            if (!file1.print(_log_array[x]))
            {
                Serial.println("Append failed");
            }
        }
    }

    file1.close();
}
void flashLOG::postlog(char *retLog, int x)
{
    sprintf(retLog, "[#%d] %s", x, _log_array[x]);
}
void flashLOG::delog(){
    SPIFFS.remove(_logfilename);
}
int flashLOG::sizelog()
{
    File file = SPIFFS.open(_logfilename, "r");
    int f = file.size();
    file.close();
    return f;
}
int flashLOG::read()
{
    int r = 0;
    int c = 0;

    File file = SPIFFS.open(_logfilename, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
    }
    while (file.available())
    {
        char tt = file.read();
        if (tt != _EOL)
        {
            _log_array[r][c] = tt;
            c++;
        }
        else
        {
            _log_array[r][c] = _EOL;
            r++;
            c = 0;
        }
    }
    file.close();
    return r;
}