
#ifndef myRF24_h
#define myRF24_h

#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <ArduinoJson.h>

class myRF24
{
    private:
        char _devname[32];
        struct RFmsg
        {
            byte msg_num;
            byte tot_msgs;
            char payload[30];
        };
        RFmsg payload;
        const byte addresses[4][6] = {"00001", "00002", "00003", "00004"};

        RF24 radio;

    public:
        myRF24(int CE_PIN, int CSN_PIN);
        void startRF24(const byte &w_addr, const byte &r_addr, const char *devname);
        bool RFwrite(const char *msg, const char *key = nullptr);
        bool RFread(char out[] = nullptr, const char *key = nullptr, unsigned long fail_micros = 200000);
        void splitMSG(const char *msg, const int arraySize, const int len);
        bool readsplit();
        void RFans();
};
#endif