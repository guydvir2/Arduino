#include <myLOG.h>

flashLOG::flashLOG(char *filename)
{
    _logfilename = filename;
}
void flashLOG::start(int max_entries, int max_entry_len)
{
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS mount failed");
    }
    _logsize = max_entries;
    _log_length = max_entry_len;
}
void flashLOG::write(const char *message)
{
    char a[_log_length+3];
    char b[_log_length+3];

    char *tfile = "/tempfile.txt";
    int num_lines = getnumlines();
    // Serial.print("lines at origin file:");
    // Serial.println(num_lines);

    File file2 = SPIFFS.open(tfile, "w");
    if (!file2)
    {
        Serial.println("Failed to open file for appending");
    }
    else
    {
        strncpy(b, message, _log_length);
        sprintf(a, "%s%c", b, _EOL);
        // Serial.print("inserting line: ");
        // Serial.println(a);
        file2.print(a);
        if (num_lines > 0)
        {
            for (int x = 0; x < min(num_lines, _logsize); x++)
            {
                if (readline(x, a))
                {
                    // Serial.printf("line %d:%s\n", x, a);
                    file2.print(a);
                }
                else
                {
                    Serial.println("Append failed");
                }
            }
        }
    }
    file2.close();
    SPIFFS.remove(_logfilename);
    SPIFFS.rename(tfile, _logfilename);
}
void flashLOG::delog()
{
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
    // int r = 0;
    // int c = 0;

    // File file = SPIFFS.open(_logfilename, "r");
    // if (!file)
    // {
    //     Serial.println("Failed to open file for reading");
    // }
    // while (file.available())
    // {
    //     char tt = file.read();
    //     if (tt != _EOL)
    //     {
    //         _log_array[r][c] = tt;
    //         c++;
    //     }
    //     else
    //     {
    //         _log_array[r][c] = _EOL;
    //         r++;
    //         c = 0;
    //     }
    // }
    // file.close();
    // return r;
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
        if (tt == _EOL && row_counter !=r)
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
    if (!file)
    {
        Serial.println("Failed to open file for reading");
    }
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