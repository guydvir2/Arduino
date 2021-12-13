#define swPin 0
#define relayPin 12
#define ledPin 13
#define RelayOn HIGH /* Relay contacts . Red Led turns on ( in hardware ) */
#define swON LOW     /* Press Button */
#define ledON LOW    /* green led will be on when Relay is off */
#define VER "powerPlug_V0.2"

bool relayState = !RelayOn;
bool ledState = !ledON;
bool OnatBoot = false;

/*
 ~~~~~ SONOFF HARDWARE ~~~~~
 #define RELAY1 12
 #define RELAY2 5
 #define INPUT1 0  // 0 for onBoard Button
 #define INPUT2 14 // 14 for extButton
 #define indic_LEDpin 13
 */
