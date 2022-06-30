#include <Arduino.h>
#define MSG_LEN 10
#define DEVNAME_LEN 8

const char *devName = "TEST1234567890";

struct RFmsg
{
    int msg_num;
    int tot_msgs;
    int tot_len;
    char payload[MSG_LEN + 1];
    char dev_name[DEVNAME_LEN + 1];
};

void newMSG(char *msg)
{
    RFmsg _payload;
    erase_struct(_payload);

    _payload.tot_len = strlen(msg);
    strncpy(_payload.dev_name, devName, DEVNAME_LEN - 1);
    _payload.tot_msgs = _payload.tot_len / MSG_LEN;

    if (_payload.tot_len % MSG_LEN > 0)
    {
        _payload.tot_msgs++;
    }

    for (uint8_t i = 0; i < _payload.tot_msgs; i++)
    {
        strcpy(_payload.payload, "");
        for (uint8_t n = 0; n < MSG_LEN; n++)
        {
            _payload.payload[n] = (char)msg[n + i * MSG_LEN];
        }
        _payload.payload[MSG_LEN] = '\0';

        Serial.print("chunk #");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(_payload.payload);
    }
    printStrcut_content(_payload);
}
void printStrcut_content(RFmsg &_payload)
{
    Serial.print("LEN: ");
    Serial.println(_payload.tot_len);
    Serial.print("PAyLOAD: ");
    Serial.println(_payload.payload);
    Serial.print("name: ");
    Serial.println(_payload.dev_name);
    Serial.print("tot_msgs: ");
    Serial.println(_payload.tot_msgs);
}
void erase_struct(RFmsg &_payload)
{
    _payload.tot_len = 0;
    _payload.tot_msgs = 0;
    strcpy(_payload.payload, "");
    strcpy(_payload.dev_name, "");
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("\nStart");
}
void loop()
{
    newMSG("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    delay(5000);
}
