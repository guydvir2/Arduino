#include <ArduinoJson.h>

// ********** Sketch Services  ***********
#define VER "WEMOS_0.1"
#define RelayOn LOW
#define USE_BOUNCE_DEBUG false
#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 300

int AutoelayOff_timeout;
const int deBounceInt = 50;

//~~~~ Switches IOs~~~~~~
int outputUpPin;
int outputDownPin;
int relayUpPin;
int relayDownPin;
