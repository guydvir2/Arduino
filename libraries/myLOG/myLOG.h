/* This library creates log file daved on Flash memory on ESP8266/ ESP32 */
/* Written by guyDvir */
/* 01/2021 */

#ifndef myLOG_h
#define myLOG_h
#include <FS.h>
#include <LittleFS.h>
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
#define TEMP_LOG_LEN 150

private:
    char *_logfilename = "/logfile.txt";
    uint8_t _buff_i = 0;
    uint8_t _logSize = TEMP_LOG_SIZE;  // entries
    uint8_t _logLength = TEMP_LOG_LEN; // chars in each entry
    bool _useDelayedSave = true;
    bool _useDebug = false;
    char _logBuffer[TEMP_LOG_SIZE][TEMP_LOG_LEN]; // Temp buffer for delayed write
    const char _EOL = '\n';

public:
    char *VeR = "flashLOG v1.1";
    unsigned long lastUpdate = 0;

private:
    bool _chkFileOK(File &_file);
    bool _write2file();
    bool _del_lines(uint8_t line_index);
    void _printDebug(char *msg);

public:
    flashLOG(char *filename = "/logfile.txt");
    bool start(uint8_t max_entries = 10, uint8_t max_entry_len = 100, bool delyedSave = true, bool debugmode = false);
    void write(const char *message, bool NOW = false);

    bool readline(uint8_t r, char *retLog);
    void looper(uint8_t savePeriod = 10);

    void rawPrintfile();
    bool delog();
    bool del_last_record();
    bool del_line(uint8_t line_index);
    uint8_t sizelog();
    uint8_t getnumlines();
};
#endif
