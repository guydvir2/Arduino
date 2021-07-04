/* This library creates log file daved on Flash memory on ESP8266/ ESP32 */
/* Written by guyDvir */
/* 01/2021 */

#ifndef myLOG_h
#define myLOG_h
#include <FS.h>
#if defined(ESP32)
#include <SPIFFS.h>
#define isESP32 true
#define isESP8266 false
#elif defined(ARDUINO_ARCH_ESP8266)
#define isESP32 false
#define isESP8266 true

#endif

class flashLOG
{
/* Following definitions are only for buffer's size, It has nothing to do with actual LOG on flash*/
#define TEMP_LOG_SIZE 10
#define TEMP_LOG_LEN 250

private:
    char *_logfilename = "/logfile.txt";
    byte _buff_i = 0;
    byte _logSize = TEMP_LOG_SIZE;  // entries
    byte _logLength = TEMP_LOG_LEN; // chars in each entry
    bool _useDelayedSave = true;
    bool _useDebug = false;
    char _logBuffer[TEMP_LOG_SIZE][TEMP_LOG_LEN]; // Temp buffer for delayed write
    const char _EOL = '\r';

public:
    char *VeR = "flashLOG v0.91";
    unsigned long lastUpdate = 0;

private:
    void _write2file();
    void _del_lines(byte line_index);
    void _printDebug(char *msg);

public:
    flashLOG(char *filename = "/logfile.txt");
    bool start(int max_entries = 10, int max_entry_len = 100, bool delyedSave = true, bool debugmode = false);
    void delog();
    void writeNow();
    void write(const char *message, bool NOW = false);
    void looper(int savePeriod = 10);
    void rawPrintfile();
    bool del_last_record();
    bool del_line(byte line_index);
    bool readline(int r, char *retLog);
    int sizelog();
    int getnumlines();
};
#endif
