#define swPin 0
#define relayPin 12
#define ledPin 13
#define relayON LOW
#define swON LOW
#define ledON LOW

bool relayState = !relayON;
bool ledState = !ledON;

// ********** Sketch Services  ***********
#define VER "powerPlug_V0.1"
static const int maxSW = 2;

bool usePWM;
bool useExtTrig;
int numSW;
int outputPin[maxSW];
int inputPin[maxSW];
int extTrigPin;
int hRebbots[maxSW];
char SW_Names[maxSW][30];

/*
 ~~~~~ SONOFF HARDWARE ~~~~~
 #define RELAY1 12
 #define RELAY2 5
 #define INPUT1 0  // 0 for onBoard Button
 #define INPUT2 14 // 14 for extButton
 #define indic_LEDpin 13
 */


