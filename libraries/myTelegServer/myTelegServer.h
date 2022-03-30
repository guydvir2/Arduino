#ifndef myTelegServer_h
#define myTelegServer_h

#include <Arduino.h>
#include "secretsIOT8266.h"

// Telegram libraries
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// define generic functiobs
typedef void (*cb_func2)(String &msg1, String &msg2, String &msg3, String &msg4);

class myTelegram
{
public:
    char *chatID;

private:
    WiFiClientSecure client;
    UniversalTelegramBot bot;
    cb_func2 _ext_func;

private:
    char *_botTok;
    uint8_t _Bot_mtbs = 2;           // mean time between scan messages in sec
    unsigned long _Bot_lasttime = 0; // last time messages' scan has been done

private:
    void handleNewMessages(int numNewMessages);

public:
    myTelegram(char *botTok, uint8_t checkServer_interval);
    void begin();
    void begin(cb_func2 funct);
    void send_msg(char *msg);
    void send_msg(String &msg);
    void looper();
};
#endif