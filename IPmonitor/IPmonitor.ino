#include <Arduino.h>
#include <myIOT2.h>
#include <myIPmonitor.h>

#define CHECK_MQTT true
#define CHECK_WIFI true
#define CHECK_INTERNET true
#define CHECK_HASS false

#define ROUTER_IP "192.168.2.1"
#define MQTT_SERVER_IP "192.168.2.101"
// #define MQTT_SERVER_IP "192.168.3.200"
#define HASS_IP "192.168.3.199"
#define INTERNET_IP "www.google.com"
// #define ROUTER_IP "192.168.3.1"

myIOT2 iot;
#define DEV_TOPIC "networkMonitor"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

#if CHECK_MQTT
#define MQTT_NAME "MQTT_Broker"
IPmonitoring MQTT_service(MQTT_SERVER_IP, MQTT_NAME);
bool pingMQTT(char *a, byte b)
{
        return iot.checkInternet(MQTT_SERVER_IP);
}
#endif
#if CHECK_WIFI
#define WIFI_NAME "Router"
IPmonitoring WiFi_service(ROUTER_IP, WIFI_NAME);
bool pingWiFi(char *a, byte b)
{
        return iot.checkInternet(ROUTER_IP);
}
#endif
#if CHECK_INTERNET
#define INTERNET_NAME "Internet"
IPmonitoring Internet_service(INTERNET_IP, INTERNET_NAME);
bool pingINTERNET(char *a, byte b)
{
        return iot.checkInternet(INTERNET_IP);
}
#endif
#if CHECK_HASS
#define HASS_NAME "HomeAssistant"
IPmonitoring HASS_service(HASS_IP, HASS_NAME);
bool pingHASS(char *a, byte b)
{
        return iot.checkInternet(HASS_IP);
}
#endif

void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        if (strcmp(incoming_msg, "status") == 0)
        {
                char a[4][30];
                char MSG[150];
#if CHECK_MQTT
                sprintf(a[0], "%s [%s] ", MQTT_NAME, CHECK_MQTT ? "[ON]" : "[OFF]");
                strcat(MSG, a[0]);
#endif
#if CHECK_WIFI
                sprintf(a[1], "%s [%s] ", WIFI_NAME, CHECK_WIFI ? "[ON]" : "[OFF]");
                strcat(MSG, a[1]);
#endif
#if CHECK_INTERNET
                sprintf(a[2], "%s [%s] ", INTERNET_NAME, CHECK_INTERNET ? "[ON]" : "[OFF]");
                strcat(MSG, a[2]);
#endif
#if CHECK_HASS
                sprintf(a[3], "%s [%s]", HASS_NAME, CHECK_HASS ? "[ON]" : "[OFF]");
                strcat(MSG, a[3]);
#endif
                iot.pub_msg(MSG);
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
                sprintf(msg, "ver #2: [%s]", WiFi_service.libVer);
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
        iot.useResetKeeper = false;
        iot.useextTopic = false;
        iot.resetFailNTP = true;
        iot.useDebug = false;
        iot.debug_level = 0;
        iot.useNetworkReset = true;
        iot.noNetwork_reset = 5;
        iot.useAltermqttServer = false;
        strcpy(iot.deviceTopic, DEV_TOPIC);
        strcpy(iot.prefixTopic, PREFIX_TOPIC);
        strcpy(iot.addGroupTopic, GROUP_TOPIC);
        // iot.start_services(addiotnalMQTT);
        iot.start_services(addiotnalMQTT,"dvirz_iot","GdSd13100301",MQTT_USER,MQTT_PASS,MQTT_SERVER_IP);
}

void setup()
{
        startIOTservices();
#if CHECK_MQTT
        MQTT_service.start(pingMQTT);
        // MQTT_service.getStatus(24);
#endif
#if CHECK_WIFI
        WiFi_service.start(pingWiFi);
        // WiFi_service.getStatus();
#endif

#if CHECK_INTERNET
        Internet_service.start(pingINTERNET);
#endif
#if CHECK_HASS
        HASS_service.start(pingHASS);
#endif
}
void loop()
{
        iot.looper();
#if CHECK_MQTT
        MQTT_service.loop();
#endif
#if CHECK_WIFI
        WiFi_service.loop();
#endif
#if CHECK_INTERNET
        Internet_service.loop();
#endif
#if CHECK_HASS
        HASS_service.loop();
#endif
        delay(100);
}
