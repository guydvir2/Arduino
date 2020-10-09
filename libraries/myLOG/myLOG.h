#ifndef myLOG_h
#define myLOG_h
#include <FS.h>

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
    char _logBuffer[LOG_SIZE][LOG_LEN];
    const char _EOL = '\r';

    void _del_lines(byte line_index = 0);
    void _write2file();

public:
    char *VeR = "flashLOG v0.5";
    long lastUpdate = 0;
    flashLOG(char *filename = "/logfile.txt"); //, int log_size = LOG_SIZE, int log_length = LOG_LEN);
    void start(int max_entries = 10, int max_entry_len = 100, bool delyedSave = true);
    void write(const char *message);
    void delog();
    int sizelog();
    bool readline(int r, char *retLog);
    int getnumlines();
    void looper(int savePeriod= 10);
    void writeNow();
};
#endif
