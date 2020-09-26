#ifndef myLOG_h
#define myLOG_h
#include <FS.h>

class flashLOG
{

#define LOG_SIZE 10
#define LOG_LEN 150

private:
    char *_logfilename = "/logfile.txt";
    int _logsize = LOG_SIZE;   // entries
    int _log_length = LOG_LEN; // chars in each entry
    // char _log_array[LOG_SIZE][LOG_LEN];
    const char _EOL = '\r';

public:
    flashLOG(char *filename = "/logfile.txt"); //, int log_size = LOG_SIZE, int log_length = LOG_LEN);
    void start(int max_entries = 10, int max_entry_len = 100);
    void write(const char *message);
    void overWrite(const char *message, int cell);
    void delog();
    int sizelog();
    int read();
    bool readline(int r, char *retLog);
    int getnumlines();
};
#endif
