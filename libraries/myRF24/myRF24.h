
#ifndef myRF24_h
#define myRF24_h

#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <ArduinoJson.h>

class myRF24
{
#define VER "myRF24_v0.1"
private:
    struct RFmsg
    {
        byte msg_num;
        byte tot_msgs;
        byte tot_len;
        char payload[22];
        char dev_name[6];
    };
    char _devname[32];
    const byte addresses[4][6] = {"00001", "00002", "00003", "00004"};

    RF24 radio;

public:
    bool debug_mode = false;

public:
    myRF24(int CE_PIN, int CSN_PIN);
    void startRF24(const byte &w_addr, const byte &r_addr, const char *devname);
    bool RFwrite(const char *msg);                                          /*plain sending*/
    bool RFwrite(const char *msg, const char *key);                         /* JSON format */
    bool RFwrite(const char *msg, const int arraySize, const int len = 20); /* long & splitted messages */
    bool RFread(char out[], int fail_micros = 200);                         /*plain read*/
    bool RFread(char out[], const char *key, int fail_micros = 200);        /*JSON format */
    bool RFread2(char outmsg[]);
    void genJSONmsg(char a[], const char *msg_t, const char *key, const char *value);

private:
    bool _wait4Rx(int timeFrame = 200);
    void _printStruct(RFmsg &msg);
};

#endif