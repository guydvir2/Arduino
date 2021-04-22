#include <Arduino.h>
#include <myIOT2.h>
#include <myIPmonitor.h>
#include <myDisplay.h>

#define CHECK_MQTT true
#define CHECK_WIFI true
#define CHECK_INTERNET true
#define CHECK_HASS true

// #define ROUTER_IP "192.168.2.1"
// #define MQTT_SERVER_IP "192.168.2.101"
#define MQTT_SERVER_IP "192.168.3.200"
#define HASS_IP "192.168.3.199"
#define INTERNET_IP "www.google.com"
#define ROUTER_IP "192.168.3.1"

#define DEV_TOPIC "networkMonitor"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

#define BUTTON_PIN D3
#define BUTTON_PRESSED LOW
#define MAX_BUTTON_PRESSES 4

myIOT2 iot;
myOLED OLED;

#if CHECK_MQTT
#define MQTT_NAME "MQTT"
IPmonitoring MQTT_service(MQTT_SERVER_IP, MQTT_NAME);
bool pingMQTT(char *a, byte b)
{
        return iot.checkInternet(MQTT_SERVER_IP);
}
#endif
#if CHECK_WIFI
#define WIFI_NAME "WiFi"
IPmonitoring WiFi_service(ROUTER_IP, WIFI_NAME);
bool pingWiFi(char *a, byte b)
{
        return iot.checkInternet(ROUTER_IP);
}
#endif
#if CHECK_INTERNET
#define INTERNET_NAME "WAN"
IPmonitoring Internet_service(INTERNET_IP, INTERNET_NAME);
bool pingINTERNET(char *a, byte b)
{
        return iot.checkInternet(INTERNET_IP);
}
#endif
#if CHECK_HASS
#define HASS_NAME "HASS"
IPmonitoring HASS_service(HASS_IP, HASS_NAME);
bool pingHASS(char *a, byte b)
{
        return iot.checkInternet(HASS_IP);
}
#endif

byte but_press_counter = 0;
void start_button()
{
        pinMode(BUTTON_PIN, INPUT);
}
void read_button()
{
        if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED)
        {
                delay(50);
                if (digitalRead(BUTTON_PIN) == BUTTON_PRESSED)
                {
                        if (but_press_counter < MAX_BUTTON_PRESSES)
                        {
                                but_press_counter++;
                        }
                        else
                        {
                                but_press_counter = 0;
                        }
                }
        }
}
void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        if (strcmp(incoming_msg, "status") == 0)
        {
                char a[4][30];
                char MSG[150];
#if CHECK_MQTT
                sprintf(a[0], "%s [%s] ", MQTT_service.nick, CHECK_MQTT ? "[ON]" : "[OFF]");
                strcat(MSG, a[0]);
#endif
#if CHECK_WIFI
                sprintf(a[1], "%s [%s] ", WiFi_service.nick, CHECK_WIFI ? "[ON]" : "[OFF]");
                strcat(MSG, a[1]);
#endif
#if CHECK_INTERNET
                sprintf(a[2], "%s [%s] ", Internet_service.nick, CHECK_INTERNET ? "[ON]" : "[OFF]");
                strcat(MSG, a[2]);
#endif
#if CHECK_HASS
                sprintf(a[3], "%s [%s]", HASS_service.nick, CHECK_HASS ? "[ON]" : "[OFF]");
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
        iot.useNetworkReset = false; // <-- only for this device
        iot.noNetwork_reset = 5;
        iot.useAltermqttServer = false;
        strcpy(iot.deviceTopic, DEV_TOPIC);
        strcpy(iot.prefixTopic, PREFIX_TOPIC);
        strcpy(iot.addGroupTopic, GROUP_TOPIC);
        iot.start_services(addiotnalMQTT);
        // iot.start_services(addiotnalMQTT, "dvirz_iot", "GdSd13100301", MQTT_USER, MQTT_PASS, MQTT_SERVER_IP);
}
void startIPmonitorings()
{
#if CHECK_MQTT
        MQTT_service.start(pingMQTT);
#endif
#if CHECK_WIFI
        WiFi_service.start(pingWiFi);
#endif

#if CHECK_INTERNET
        Internet_service.start(pingINTERNET);
#endif
#if CHECK_HASS
        HASS_service.start(pingHASS);
#endif
}
void startOLED()
{
        OLED.start();
        // OLED.CenterTXT("BootUp", "IPmonitoring");
}
bool check_all_services()
{
        byte inc = 0;
        byte max_v = 0;
        byte all_service_ok = 1;

#if CHECK_MQTT
        inc++;
        all_service_ok += MQTT_service.isConnected * pow(2, inc);
#endif
#if CHECK_WIFI
        inc++;
        all_service_ok += WiFi_service.isConnected * pow(2, inc);
#endif
#if CHECK_INTERNET
        inc++;
        all_service_ok += Internet_service.isConnected * pow(2, inc);
#endif
#if CHECK_HASS
        inc++;
        all_service_ok += HASS_service.isConnected * pow(2, inc);
#endif
        for (int i = 1; i <= inc; i++)
        {
                max_v += pow(2, i);
        }
        if (all_service_ok == max_v)
        {
                return 1;
        }
        else
        {
                return 0;
        }
}
void display_boot()
{
        char txt[4][12];
        char uptime[12];
        char upday[5];
        iot.convert_epoch2clock(now(), WiFi_service.bootClk, uptime, upday);

        sprintf(txt[0], "BootUp");
        sprintf(txt[1], "%s", WiFi_service.libVer);
        iot.return_clock(txt[2]);
        iot.return_date(txt[3]);
        OLED.CenterTXT(txt[0], txt[1], txt[2], uptime);
}
void compose_OLEDtxt0(byte &x, char a[], IPmonitoring &ipmon)
{
        sprintf(a, "%s %s", ipmon.nick, ipmon.isConnected ? "V" : "x");
        x++;
}
void display_OK()
{
        char txt[4][10];

        for (int n = 0; n < 4; n++)
        {
                sprintf(txt[n], "");
        }
        byte inc = 0;

        if (check_all_services)
        {
                sprintf(txt[0], "All OK");
                iot.convert_epoch2clock(now(), WiFi_service.bootClk, txt[3], txt[2]);
                // iot.return_clock(txt[2]);
                // iot.return_date(txt[3]);
        }
        else
        {
#if CHECK_MQTT
                compose_OLEDtxt0(inc, txt[inc], MQTT_service);
#endif
#if CHECK_WIFI
                compose_OLEDtxt0(inc, txt[inc], WiFi_service);

#endif
#if CHECK_INTERNET
                compose_OLEDtxt0(inc, txt[inc], Internet_service);
#endif
#if CHECK_HASS
                compose_OLEDtxt0(inc, txt[inc], HASS_service);
#endif
        }
        OLED.CenterTXT(txt[0], txt[1], txt[2], txt[3]);
}
void display_1()
{
}
void display_stats(byte &i)
{
}
void displays_looper()
{
        static unsigned long disp_t = 0;
        const byte Sec_boot_display = 25;
        if (millis() < Sec_boot_display * 1000)
        {
                display_boot();
        }
        else
        {
                if (but_press_counter == 0)
                {
                        if (check_all_services) // All Sevices are OK
                        {
                                display_OK();
                        }
                }
                else
                {
                }
        }
}
void setup()
{
        start_button();
        startIOTservices();
        startIPmonitorings();
        startOLED();
        check_all_services();
}
void loop()
{
        iot.looper();
        displays_looper();
        read_button();
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
