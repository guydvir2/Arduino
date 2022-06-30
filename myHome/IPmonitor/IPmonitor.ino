#include <myIOT2.h>
#include <myIPmonitor.h>

#define CHECK_MQTT true
#define CHECK_HASS true
#define CHECK_WIFI true
#define CHECK_INTERNET true
#define USE_DISPLAY false

#include "IPmonitoring.h"
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
