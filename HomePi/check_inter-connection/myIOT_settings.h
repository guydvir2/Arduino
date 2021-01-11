#include <myIOT2.h>

#define DEV_TOPIC "internetMonitor"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

myIOT2 iot;

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    if (strcmp(incoming_msg, "status") == 0)
    {
        char days_char[2][20];
        char times_char[2][20];
        char datestr[50];
        time_t moni = readFlog(connectionLOG, 0);
        convert_epoch2clock(now(), moni, times_char[1], days_char[1]);
        epoch2datestr(moni, datestr);
        convert_epoch2clock(now(), readFlog(disconnectionLOG, disconnectionLOG.getnumlines() - 1), times_char[2], days_char[2]);
        sprintf(msg, "Status: Internet[%s] MQTTserver[%s] HomeAssistant[%s] Disconnects[%d] monitoringTime [%s %s], First log[%s] onTime[%s %s]",
                internetConnected ? "OK" : "FAIL", mqttConnected ? "OK" : "FAIL", homeAssistantConnected ? "OK" : "FAIL",
                disconnectionLOG.getnumlines() - 1, days_char[1], times_char[1], datestr, days_char[2], times_char[2]);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2: [%s], LCDdisplay[%d]", VER, USE_DISPLAY);
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help #2: Commands #3 - [disconnects,[x],[y] {[x=num] [y=h,d,w]}, disconnects_log, del_disc_log]");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "del_disc_log") == 0)
    {
        deleteFlog(connectionLOG);
        deleteFlog(disconnectionLOG);
        iot.pub_log("Connection logs - Deleted");
        iot.sendReset("logs deletion");
    }
    else if (strcmp(incoming_msg, "disconnects_log") == 0)
    {
        char log[100];
        char tstamp[20];

        iot.pub_debug("~~~Start~~~");
        if (internetConnected)
        {
            for (int i = 0; i < disconnectionLOG.getnumlines(); i++)
            {
                epoch2datestr(readFlog(disconnectionLOG, i), tstamp);
                sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(connectionLOG, i) - readFlog(disconnectionLOG, i));
                iot.pub_debug(log);
            }
        }
        else
        {
            for (int i = 0; i < connectionLOG.getnumlines(); i++)
            {
                epoch2datestr(readFlog(disconnectionLOG, i), tstamp);
                sprintf(log, "[%d]: [%s], duration [%d]sec", i, tstamp, readFlog(connectionLOG, i) - readFlog(disconnectionLOG, i));
                iot.pub_debug(log);
            }
        }
        iot.pub_msg("log extracted");
        iot.pub_debug("~~~End~~~");
    }

    // ±±±±±±±±±± MQTT MSGS ±±±±±±±±±±±±
    else
    {
        int num_parameters = iot.inline_read(incoming_msg);
        if (strcmp(iot.inline_param[0], "disconnects") == 0 && (strcmp(iot.inline_param[2], "h") == 0 || strcmp(iot.inline_param[2], "d") == 0 || strcmp(iot.inline_param[2], "w")))
        {
            const int d = 24;
            const int w = d * 7;

            int counter = 0;
            int conTime = 0;
            int disconTime = 0;
            char time_chars[3][20];
            char days_chars[3][20];

            if (strcmp(iot.inline_param[2], "h") == 0)
            {
                calc2(conTime, disconTime, counter, atoi(iot.inline_param[1]));
            }
            else if (strcmp(iot.inline_param[2], "d") == 0)
            {
                calc2(conTime, disconTime, counter, d * atoi(iot.inline_param[1]));
            }
            else if (strcmp(iot.inline_param[2], "w") == 0)
            {
                calc2(conTime, disconTime, counter, w * atoi(iot.inline_param[1]));
            }

            convert_epoch2clock(disconTime, 0, time_chars[1], days_chars[1]);

            sprintf(msg, "Disconnects: lookupTime [%d%s] disconnectCounter [%d], offline [%s %s] ratio [%.1f%%]",
                    atoi(iot.inline_param[1]), iot.inline_param[2], counter, days_chars[1],
                    time_chars[1], 100.0 * (float)conTime / ((float)disconTime + (float)conTime));
            iot.pub_msg(msg);

            // int x = connectionLOG.getnumlines();
            // int y = disconnectionLOG.getnumlines();
            // Serial.println("ConnectionLog");
            // for (int i = 0; i < x; i++)
            // {
            //         Serial.printf("%d: %d\n", i, readFlog(connectionLOG, i));
            // }
            // Serial.println("DisConnectionLog");
            // for (int i = 0; i < y; i++)
            // {
            //         Serial.printf("%d: %d\n", i, readFlog(disconnectionLOG, i));
            // }
        }
    }
}
void startIOTservices()
{
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = false;
    iot.useextTopic = false;
    iot.resetFailNTP = true;
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

    iot.start_services(addiotnalMQTT);
}



