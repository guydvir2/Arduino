#ifndef myLOG_h
#define myLOG_h

#include <FS.h>

class flashLOG
{

#define LOG_SIZE 100
#define LOG_LEN 250

private:
    char *_logfilename = "/logfile.txt";
    const static int _logsize = LOG_SIZE;     // entries
    const static int _log_length = LOG_LEN; // chars in each entry
    char _log_array[_logsize][_log_length];
    const char _EOL = '\r';

public:
    flashLOG(char *filename = "/logfile.txt");//, int log_size = LOG_SIZE, int log_length = LOG_LEN);
    void start();
    void write(const char *message);
    void postlog(char *retLog, int x=9999);
    void delog();
    int sizelog();
    int read();

};
#endif
