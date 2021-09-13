#include <myIOT2.h>
#include <myIPmonitor.h>

#define CHECK_WIFI true
#define CHECK_INTERNET true
#define CHECK_MQTT true
#define CHECK_HASS false
#define USE_DISPLAY false

#define MQTT_SERVER_IP "192.168.3.200"
#define HASS_IP "192.168.3.199"
#define INTERNET_IP "www.google.com"
#define ROUTER_IP "192.168.3.1"

#define DEV_TOPIC "networkMonitor"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

myIOT2 iot;

#if CHECK_MQTT
#define MQTT_NAME "MQTT"
IPmonitoring MQTT_service(MQTT_SERVER_IP, MQTT_NAME);
bool pingMQTT(char *a, uint8_t b)
{
        return iot.checkInternet(MQTT_SERVER_IP);
}
void MQTToutbox(char *msg)
{
        iot.pub_msg(msg);
}
#endif
#if CHECK_WIFI
#define WIFI_NAME "WiFi"
IPmonitoring WiFi_service(ROUTER_IP, WIFI_NAME);
bool pingWiFi(char *a, uint8_t b)
{
        return iot.checkInternet(ROUTER_IP);
}
#endif
#if CHECK_INTERNET
#define INTERNET_NAME "WAN"
IPmonitoring Internet_service(INTERNET_IP, INTERNET_NAME);
bool pingINTERNET(char *a, uint8_t b)
{
        return iot.checkInternet(INTERNET_IP);
}
#endif
#if CHECK_HASS
#define HASS_NAME "HASS"
IPmonitoring HASS_service(HASS_IP, HASS_NAME);
bool pingHASS(char *a, uint8_t b)
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

void startIPmonitorings()
{
#if CHECK_MQTT
        MQTT_service.start(pingMQTT, MQTToutbox);
#endif
#if CHECK_WIFI
        WiFi_service.start(pingWiFi, MQTToutbox);
#endif

#if CHECK_INTERNET
        Internet_service.start(pingINTERNET, MQTToutbox);
#endif
#if CHECK_HASS
        HASS_service.start(pingHASS, MQTToutbox);
#endif
}
bool check_all_services()
{
        uint8_t inc = 0;
        uint8_t max_v = 0;
        uint8_t all_service_ok = 1;

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
void service_loop(IPmonitoring &s)
{
        s.loop();
}
void all_services_loop()
{
#if CHECK_MQTT
        service_loop(MQTT_service);
#endif
#if CHECK_WIFI
        service_loop(WiFi_service);
#endif
#if CHECK_INTERNET
        service_loop(Internet_service);
#endif
#if CHECK_HASS
        service_loop(HASS_service);
#endif
}

#if USE_DISPLAY
#include <myDisplay.h>

#define BUTTON_PIN 5
#define BUTTON_PRESSED LOW
#define MAX_BUTTON_PRESSES 4

myOLED OLED;
uint8_t but_press_counter = 0;
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
void startOLED()
{
        OLED.start();
        // OLED.CenterTXT("BootUp", "IPmonitoring");
}
void display_boot()
{
        char txt[4][12];
        char uptime[12];
        char upday[5];
        iot.convert_epoch2clock(iot.now(), WiFi_service.bootClk, uptime, upday);

        sprintf(txt[0], "BootUp");
        sprintf(txt[1], "%s", WiFi_service.libVer);
        iot.return_clock(txt[2]);
        iot.return_date(txt[3]);
        OLED.CenterTXT(txt[0], txt[1], txt[2], uptime);
}
void compose_OLEDtxt0(uint8_t &x, char a[], IPmonitoring &ipmon)
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
        uint8_t inc = 0;

        if (check_all_services)
        {
                sprintf(txt[0], "All OK");
                iot.convert_epoch2clock(iot.now(), WiFi_service.bootClk, txt[3], txt[2]);
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
void display_stats(uint8_t &i)
{
}
void displays_looper()
{
        static unsigned long disp_t = 0;
        const uint8_t Sec_boot_display = 25;
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
#endif

void setup()
{
        startIOTservices();
        startIPmonitorings();
#if USE_DISPLAY
        startOLED();
        start_button();
#endif
        // check_all_services();
}
void loop()
{
        iot.looper();
        all_services_loop();
#if USE_DISPLAY
        displays_looper();
        read_button();
#endif
}
