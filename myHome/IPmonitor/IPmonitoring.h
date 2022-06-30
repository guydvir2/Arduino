#define MQTT_SERVER_IP "192.168.2.200"
#define HASS_IP "192.168.2.199"
#define INTERNET_IP "www.google.com"
#define ROUTER_IP "192.168.2.1"

extern myIOT2 iot;
extern bool MQTToutbox(char *msg, uint8_t msg_t);

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

bool pingTo(char *dest, uint8_t b)
{
    return iot.checkInternet(dest, b);
}
void get_serviceStatus(IPmonitoring &s, char str[])
{
    sprintf(str, "%s[%s] ", s.nick, s.connState ? "V" : "X");
}
void startIPmonitorings()
{
#if CHECK_MQTT
    MQTT_service.start(pingTo, MQTToutbox);
#endif
#if CHECK_WIFI
    WiFi_service.start(pingTo, MQTToutbox);
#endif
#if CHECK_INTERNET
    Internet_service.start(pingTo, MQTToutbox);
#endif
#if CHECK_HASS
    HASS_service.start(pingTo, MQTToutbox);
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