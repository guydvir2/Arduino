#include <mySerialMSG.h>
#include <ArduinoJson.h>

mySerialMSG::mySerialMSG(const char *name, Stream &stream) : _name(name), _serial(stream)
{
}
void mySerialMSG::start(cb_func3 cb)
{
    _external_cb = cb;
}
void mySerialMSG::loop()
{
    _incoming_looper();
    _pinger_looper();
}
void mySerialMSG::sendMsg(char *kargs[], uint8_t s)
{
    StaticJsonDocument<JSON_SIZE> doc;
    for (int i = 0; i < s; i++)
    {
        doc[KW[i]] = kargs[i];
    }
    serializeJson(doc, _serial);
    TxPackets++;
}
void mySerialMSG::sendMsg(const char *karg_0, const char *karg_1, const char *karg_2, const char *karg_3)
{
    StaticJsonDocument<JSON_SIZE> doc;
    doc[KW[0]] = karg_0;
    doc[KW[1]] = karg_1;
    doc[KW[2]] = karg_2;
    doc[KW[3]] = karg_3;

    serializeJson(doc, _serial);
    TxPackets++;
}

void mySerialMSG::_gen_ping()
{
    sendMsg(_name, "system", "ping", "Tx");
}
void mySerialMSG::_send_ping()
{
    if (millis() - _last_ping > 1000 * ping_interval)
    {
        _last_ping = millis();
        _gen_ping();
    }
}
void mySerialMSG::_pinger_looper()
{
    if (usePings)
    {
        _send_ping();
        _fail_recv_ping();
        _notifications_ping();
    }
}
void mySerialMSG::_incoming_looper()
{
    if (_serial.available())
    {
        StaticJsonDocument<JSON_SIZE> doc;
        DeserializationError error = deserializeJson(doc, _serial);

        if (!error)
        {
            RxPackets++;
            if (usePings && _check_recv_ping(doc))
            {
                return;
            }
            else
            {
                _external_cb(doc);
            }
        }
        else
        {
#if DEBUG_MODE
            _debugSerial.println("BAD_Rx");
#endif
            sendMsg(_name, "system", "Rx", "error");
            failRxPackets++;
        }
    }
}
void mySerialMSG::_fail_recv_ping()
{
    const uint8_t err_interval = 1;
    if (_lastOK_ping != 0 && (millis() - _lastOK_ping) > 1000 * (ping_interval + err_interval) && pingOK)
    {
        pingOK = false;
        #if DEBUG_MODE
                _debugSerial.println("pingOK_CHANGE_TO_FAIL");
        #endif
    }
}
void mySerialMSG::_notifications_ping()
{
    if (!pingOK && !_err_notification && (_last_ping != 0 && millis() - _last_ping > (0.5 * ping_interval * 1000))) /* Notify failure*/
    {
        _err_notification = true;
#if DEBUG_MODE
        _debugSerial.println("PING_SEND_FAIL_STATUS");
#endif
        sendMsg(_name, "system", "ping", "fail");
    }
    else if (pingOK && _err_notification) /* Notify restore Ping*/
    {
        _err_notification = false;
#if DEBUG_MODE
        _debugSerial.println("PING_SEND_RESTORE_OK_STATUS");
#endif
        sendMsg(_name, "system", "ping", "OK");
    }
}
bool mySerialMSG::_check_recv_ping(JsonDocument &_doc)
{
    if (strcmp(_doc[KW[1]], "system") == 0 && strcmp(_doc[KW[2]], "ping") == 0 && strcmp(_doc[KW[3]], "Tx") == 0)
    {
        _lastOK_ping = millis();
        pingOK = true;
        return 1;
    }
    else
    {
        return 0;
    }
}