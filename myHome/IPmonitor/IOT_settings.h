#define DEV_TOPIC "networkMonitor2"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

myIOT2 iot;

bool MQTToutbox(char *msg, uint8_t msg_t)
{
        if (msg_t == 0)
        {
                iot.pub_msg(msg);
        }
        else if (msg_t == 1)
        {
                iot.pub_log(msg);
        }
        else if (msg_t == 2)
        {
                iot.pub_debug(msg);
        }
        return 1;
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        if (strcmp(incoming_msg, "status") == 0)
        {
                char t[30];
                sprintf(msg, "[Status]: ");
#if CHECK_MQTT
                get_serviceStatus(MQTT_service, t);
                strcat(msg, t);
#endif
#if CHECK_WIFI
                Serial.println("WIFI");
                get_serviceStatus(WiFi_service, t);
                strcat(msg, t);
                Serial.println(t);
#endif
#if CHECK_INTERNET
                get_serviceStatus(Internet_service, t);
                strcat(msg, t);
#endif
#if CHECK_HASS
                get_serviceStatus(HASS_service, t);
                strcat(msg, t);
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
                sprintf(msg, "Help2: Commands #3 - [report, printLOG, del_logs]");
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
        iot.useWDT = true;
        iot.useOTA = true;
        iot.useDebug = true;
        iot.useSerial = true;
        iot.useextTopic = false;
        iot.useBootClockLog = true;
        iot.useResetKeeper = false;
        iot.useNetworkReset = false; // <-- only for this device
        iot.debug_level = 0;
        iot.noNetwork_reset = 5;

        strcpy(iot.deviceTopic, DEV_TOPIC);
        strcpy(iot.prefixTopic, PREFIX_TOPIC);
        strcpy(iot.addGroupTopic, GROUP_TOPIC);
        
        iot.start_services(addiotnalMQTT);
}