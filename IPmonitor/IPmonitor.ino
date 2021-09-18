#include <myIOT2.h>
#include <myIPmonitor.h>

#define CHECK_WIFI true
#define CHECK_INTERNET true
#define CHECK_MQTT true
#define CHECK_HASS true
#define USE_DISPLAY false

#define MQTT_SERVER_IP "192.168.3.200"
#define HASS_IP "192.168.3.199"
#define INTERNET_IP "www.google.com"
#define ROUTER_IP "192.168.3.1"

myIOT2 iot;

#if CHECK_MQTT
IPmonitoring MQTT_service(MQTT_SERVER_IP, "MQTT_Broker");
#endif
#if CHECK_WIFI
IPmonitoring WiFi_service(ROUTER_IP, "WiFi_Router");
#endif
#if CHECK_INTERNET
IPmonitoring Internet_service(INTERNET_IP, "WAN");
#endif
#if CHECK_HASS
IPmonitoring HASS_service(HASS_IP, "HomeAssistant");
#endif

bool MQTToutbox(char *msg, uint8_t msg_t)
{
        // Serial.println(msg);
        // if (msg_t == 0)
        // {
        //         iot.pub_msg(msg);
        // }
        // else if (msg_t == 1)
        // {
        //         iot.pub_log(msg);
        // }
        // else if (msg_t == 2)
        // {
        //         iot.pub_debug(msg);
        // }
}
void startIPmonitorings()
{
#if CHECK_MQTT
        MQTT_service.start(iot.checkInternet, MQTToutbox);
#endif
#if CHECK_WIFI
        WiFi_service.start(iot.checkInternet, MQTToutbox);
#endif
#if CHECK_INTERNET
        Internet_service.start(iot.checkInternet, MQTToutbox);
#endif
#if CHECK_HASS
        HASS_service.start(iot.checkInternet, MQTToutbox);
#endif
}
void all_services_loop()
{
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
}

#if USE_DISPLAY
#include <myDisplay.h>
#include "OLEDdisplay.h"
#endif
#include "IOT_settings.h"

void setup()
{
        startIOTservices();
        startIPmonitorings();
#if USE_DISPLAY
        startOLED();
        start_button();
#endif
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
