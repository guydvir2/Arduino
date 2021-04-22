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

#define LOG_SIZE 10
#define LOG_LEN 200

private:
    char *_logfilename = "/logfile.txt";
    int _logsize = LOG_SIZE;   // entries
    int _log_length = LOG_LEN; // chars in each entry
    int _buff_i = 0;
    bool _useDelayedSave = true;
    char _logBuffer[LOG_SIZE][LOG_LEN]; // Temp buffer for delayed write
    const char _EOL = '\r';

public:
    char *VeR = "flashLOG v0.7";
    long lastUpdate = 0;

private:
    void _del_lines(byte line_index = 0);
    void _write2file();

public:
    flashLOG(char *filename = "/logfile.txt"); //, int log_size = LOG_SIZE, int log_length = LOG_LEN);
    bool start(int max_entries = 10, int max_entry_len = 100, bool delyedSave = true);
    void delog();
    bool del_last_record();
    bool del_line(byte line_index);
    int sizelog();
    int getnumlines();
    void writeNow();
    void write(const char *message);
    bool readline(int r, char *retLog);
    void looper(int savePeriod = 10);
    void rawPrintfile();

};
#endif
