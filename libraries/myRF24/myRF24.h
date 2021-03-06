
#ifndef myRF24_h
#define myRF24_h

#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <ArduinoJson.h>

class myRF24
{
#define VER "myRF24_v0.5"
private:
    struct RFmsg
    {
        byte msg_num;
        byte tot_msgs;
        byte tot_len;
        char payload[20];
        char dev_name[9];
    };
    char _devname[32];
    byte _w_addr;
    byte _r_addr;
    uint8_t _PA_level;
    rf24_datarate_e _Data_rate;
    int _ch;
    const byte addresses[4][6] = {"00001", "00002", "00003", "00004"};

public:
    RF24 radio;
    bool debug_mode = false;
    bool use_ack = false;

public:
    myRF24(int CE_PIN, int CSN_PIN);
    bool startRF24(const byte &w_addr, const byte &r_addr, const char *devname, uint8_t PA_level = RF24_PA_MIN, rf24_datarate_e Data_rate = RF24_1MBPS, int ch = 1);
    bool RFwrite(const char *msg, const int arraySize, const int len = 20); /* long & splitted messages */
    bool RFread(char out[], int fail_micros = 200);                         /*plain read*/
    bool RFread2(char outmsg[], int del = 100);                             /* Split Messages */
    void failDetect();
    bool resetRF24();
    void wellness_Watchdog();

private:
    bool _start();
    bool _RFwrite_nosplit(const char *msg); /*plain sending*/
    bool _wait4Rx(int timeFrame = 200);
    void _printStruct(RFmsg &msg);
};

#endif