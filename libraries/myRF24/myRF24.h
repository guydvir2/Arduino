#ifndef myRF24_h
#define myRF24_h

#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

class myRF24
{
#define VER "myRF24_v0.6"
#define MSG_LEN 20
#define DEVNAME_LEN 7
private:
    struct RFmsg
    {
        uint8_t msg_num;
        uint8_t tot_msgs;
        uint8_t tot_len;
        char payload[MSG_LEN + 1];
        char dev_name[DEVNAME_LEN + 1];
    };

    char *_devname;
    uint8_t _ch;
    uint8_t _w_addr;
    uint8_t _r_addr;
    uint8_t _PA_level;
    rf24_datarate_e _Data_rate;
    const uint8_t addresses[4][6] = {"00001", "00002", "00003", "00004"};

public:
    RF24 radio;
    bool use_ack = false;
    bool debug_mode = false;

public:
    myRF24(uint8_t CE_PIN, uint8_t CSN_PIN);
    bool startRF24(const uint8_t &w_addr, const uint8_t &r_addr, char *devname, uint8_t PA_level = RF24_PA_MIN, rf24_datarate_e Data_rate = RF24_1MBPS, uint8_t ch = 1);
    bool RFwrite(const char *msg);                  /* long & splitted messages */
    bool RFread2(char out[], int fail_micros = 200); /*plain read*/
    bool RFread(char out[], char from[], int del=100);
    bool resetRF24();

private:
    bool _start();
    void _failDetect();
    void _wellness_Watchdog();
    void _printStruct(RFmsg &msg);
    bool _wait4Rx(int timeFrame = 200);
    void _erase_struct(RFmsg &_payload);
};

#endif