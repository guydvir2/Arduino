#include <myIOT.h>

#include <FS.h>
#include <LittleFS.h>
// #include <EEPROM.h>


// flashLOG fl;

void start() {
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed");
    }
    else {
        Serial.println("MOUNTED OK");
    }
}
void read() {
    int r = 0;
    int c = 0;

    File file = LittleFS.open(_logfilename, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
    }
    else {
        while (file.available())
        {
            char tt = file.read();
            //     if (tt != '\n')
            //     {
            //         _log_array[r][c] = tt;
            //         c++;
            //     }
            //     else
            //     {
            //         _log_array[r][c] = '\n';
            //         r++;
            //         c = 0;
            //         // Serial.println(_log_array[r-1]);
            //     }
            Serial.write(tt);
        }
    }
    file.close();
    //     return r;

}
void write(const char *message)
{
    char a[_log_length];
    //     sprintf(a, "%s\n", message);

    File file = LittleFS.open(_logfilename, "a");
    if (!file)
    {
        Serial.println("Failed to open file for appending");
    }
    else {
        if (file.print(message)) {
            Serial.println("Append OK");
        }
        else {
            Serial.println("Append fail");
        }
    }
    Serial.printf("file size: %d\n", file.size());
    delay(2000);
    file.close();
}
void append(const char *message)
{
    char a[_log_length];
    //     sprintf(a, "%s\n", message);

    File file = LittleFS.open(_logfilename, "a");
    if (!file)
    {
        Serial.println("Failed to open file for appending");
    }
    else {
        if (file.print(message)) {
            Serial.println("Append OK");
        }
        else {
            Serial.println("Append fail");
        }
    }
    Serial.printf("file size: %d\n", file.size());
    //     delay(2000);
    file.close();
}
void postlog(int x)
{
    int num_lines = 0;//read();
    int y = min(x, num_lines);
    for (int a = 0; a < y; a++)
    {
        if (y > 1)
        {
            char t[_log_length];
            sprintf(t, "[#%d] %s", a, _log_array[a]);
            Serial.println(t);
        }
        else
        {
            Serial.println(_log_array[a]);
        }
    }
}
int sizelog() {
    File file = LittleFS.open(_logfilename, "r");
    int f=file.size();
    file.close();
    return f;
}

void setup() {
    Serial.begin(9600);
    // EEPROM.begin(12);
    start();
    // LittleFS.format();
    // fl.start(_logfilename);
    
    read();
    append("GUYDDVIR\n");
    
    // put your setup code here, to run once:

}

void loop() {
    if (millis()>5000) {
        ESP.restart();
    }
    delay(500);
    // put your main code here, to run repeatedly:

}
