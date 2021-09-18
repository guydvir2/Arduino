#define DEV_TOPIC "networkMonitor2"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

char *str_services(IPmonitoring &s)
{
        char *str = new char[30];
        sprintf(str, "%s[%s] ", s.nick, s.isConnected ? "V" : "X");
        return str;
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        if (strcmp(incoming_msg, "status") == 0)
        {
                sprintf(msg, "[Status]: ");
#if CHECK_MQTT
                strcat(msg, str_services(MQTT_service));
#endif
#if CHECK_WIFI
                strcat(msg, str_services(WiFi_service));

#endif
#if CHECK_INTERNET
                strcat(msg, str_services(Internet_service));

#endif
#if CHECK_HASS
                strcat(msg, str_services(HASS_service));
#endif
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "report") == 0)
        {
#if CHECK_MQTT
                MQTT_service.getStatus();
#endif
#if CHECK_WIFI
                WiFi_service.getStatus();
#endif
#if CHECK_INTERNET
                Internet_service.getStatus();
#endif
#if CHECK_HASS
                HASS_service.getStatus();
#endif
        }
        else if (strcmp(incoming_msg, "printLOG") == 0)
        {
#if CHECK_MQTT
                MQTT_service.printFlog();
#endif
#if CHECK_WIFI
                WiFi_service.printFlog();
#endif
#if CHECK_INTERNET
                Internet_service.printFlog();
#endif
#if CHECK_HASS
                HASS_service.printFlog();
#endif
        }
        else if (strcmp(incoming_msg, "ver2") == 0)
        {
                // sprintf(msg, "ver #2: [%s]", WiFi_service.libVer);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help2") == 0)
        {
                sprintf(msg, "Help #2: Commands #3 - [report, printLOG, del_logs]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "del_logs") == 0)
        {
#if CHECK_MQTT
                MQTT_service.deleteLOG();
#endif
#if CHECK_WIFI
                WiFi_service.deleteLOG();
#endif
#if CHECK_INTERNET
                Internet_service.deleteLOG();
#endif
#if CHECK_HASS
                HASS_service.deleteLOG();
#endif
                iot.pub_log("Logs deleted");
                iot.sendReset("Reset after logs deletion");
        }
}
void startIOTservices()
{
        iot.useSerial = true;
        iot.useWDT = true;
        iot.useOTA = true;
        iot.extDefine = true;
        iot.useResetKeeper = false;
        iot.useextTopic = false;
        iot.useDebug = true;
        iot.debug_level = 0;
        iot.useNetworkReset = false; // <-- only for this device
        iot.noNetwork_reset = 5;
        iot.useBootClockLog = true;
        iot.useAltermqttServer = false;
        strcpy(iot.deviceTopic, DEV_TOPIC);
        strcpy(iot.prefixTopic, PREFIX_TOPIC);
        strcpy(iot.addGroupTopic, GROUP_TOPIC);
        iot.start_services(addiotnalMQTT);
}