
#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef mySerialMSG_h
#define mySerialMSG_h

typedef void (*cb_func3)(JsonDocument &_doc);

class mySerialMSG
{
#define JSON_SIZE 256
#define DEBUG_MODE false
public:
    const char *KW[5];
    const char *ver = "v0.1";
    bool pingOK = false;
    bool usePings = false;
    uint8_t ping_interval = 2;
    unsigned int failRxPackets = 0;
    unsigned long RxPackets = 0;
    unsigned long TxPackets = 0;

private:
#if DEBUG_MODE
    Stream &_debugSerial = Serial;
#endif

    const char *_name;
    unsigned long _last_ping = 0;
    unsigned long _lastOK_ping = 0;
    const uint8_t _lastPing_clk = 0;
    bool _err_notification = false;

    cb_func3 _external_cb;
    Stream &_serial = Serial;

public:
    mySerialMSG(const char *name, Stream &stream = Serial);
    void sendMsg(char *kargs[], uint8_t s);
    void sendMsg(const char *karg_0, const char *karg_1, const char *karg_2 = nullptr, const char *karg_3 = nullptr);
    void start(cb_func3 cb);
    void loop();

private:
    void _gen_ping();
    void _send_ping();
    void _pinger_looper();
    void _fail_recv_ping();
    void _incoming_looper();
    void _notifications_ping();
    bool _check_recv_ping(JsonDocument &_doc);
};
#endif