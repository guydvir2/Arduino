#define maxW 4
#define MAX_Relays 8
#define TOPIC_LEN 40

struct SwitchStruct
{
    uint8_t id = 0;
    uint8_t RFch = 255; /* Define RF channel. leave 255 to disable */
    uint8_t outPin = 255;

    bool type;               /* <0> is ON-Off Switch, <1> push button */
    bool virtCMD = false;    /* Trigger a MQTT or a Relay */
    bool useButton = true;   /* Input is an option */
    bool guessState = false; /* when using IOcmd and a pushbutton state can not be known */
    char Topic[TOPIC_LEN];

    Button2 button; /* inputs only */
};

WinSW *winSW_V[maxW] = {nullptr, nullptr, nullptr, nullptr};
SwitchStruct *SW_v[MAX_Relays] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

char *buttMQTTcmds[2] = {"on", "off"};
const char *winMQTTcmds[3] = {"up", "down", "off"};

/* ****** Paramters that will be updated from FLASH saved on file ******* */

// <<<<<<<<<<<<<<<<<<<<<  Button Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint8_t numSW = 2;
uint8_t RFpin = 27;

#if defined(ESP32)
uint8_t buttonPins[MAX_Relays] = {19, 17, 16, 5, 0, 0, 0, 0};
uint8_t relayPins[MAX_Relays] = {25, 26, 33, 32, 0, 0, 0, 0};
#elif defined(ESP8266)
uint8_t buttonPins[MAX_Relays] = {D7, D2, D3, D4, 0, 0, 0, 0};
uint8_t relayPins[MAX_Relays] = {D8, D6, D7, D8, 0, 0, 0, 0};
#endif


bool buttonTypes[MAX_Relays] = {0, 0, 0, 1}; /* <0> is ON-Off Switch, <1> push button */
int RF_keyboardCode[] = {3135496, 3135492, 3135490, 3135489};

const char *SW_prefixTopic = "myHome/Lights/int/gFloor/";
// <<<<<<<<<<<<<<<<<<<<<  WinSW Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

uint8_t numW = 1;
#if defined(ESP32)
uint8_t WrelayPins[maxW][2] = {{33, 32}, {25, 26}, {0, 0}, {0, 0}};
uint8_t WinputPins[maxW][2] = {{16, 5}, {19, 17}, {0, 0}, {0, 0}};
uint8_t WextInPins[maxW][2] = {{5, 4}, {255, 255}, {255, 255}, {255, 255}};
#elif defined(ESP8266)
uint8_t WrelayPins[maxW][2] = {{D3, D4}, {D5, D6}, {0, 0}, {0, 0}};
uint8_t WinputPins[maxW][2] = {{D5, D6}, {D7, D8}, {0, 0}, {0, 0}};
uint8_t WextInPins[maxW][2] = {{D7, D8}, {255, 255}, {255, 255}, {255, 255}};
#endif


const char *WIN_prefixTopic = "myHome/Windows/gFloor/";

// <<<<<<<<<<<<<<<<<<<<<  IOcmds Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void updateTopics()
{
    // strcpy(winGroupTopics[0], "myHome/Windows");
    // strcpy(winGroupTopics[1], "myHome/lockdown");
    // strcpy(winGroupTopics[2], "myHome/alarmMonitor");
    // strcpy(winGroupTopics[3], "myHome/Windows/gFloor");

    // strcpy(buttGroupTopics[0], "myHome/Lights");
    // strcpy(buttGroupTopics[1], "myHome/Lights/int");
    // strcpy(buttGroupTopics[2], "myHome/Lights/int/gFloor");
}
/*************************************************************************/