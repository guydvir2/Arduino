#define maxW 4
#define MAX_Relays 8

/* ****** Paramters that will be updated from FLASH saved on file ******* */

// <<<<<<<<<<<<<<<<<<<<<  Button Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool useRF = true;
bool useButton = true;

uint8_t numSW = 1;
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
char *ButtonNames[MAX_Relays] = {"Saloon", "Entrance", "Parking", "Kitchen", "NAN", "NAN", "NAN", "NAN"};

// <<<<<<<<<<<<<<<<<<<<<  WinSW Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

uint8_t numW = 1;
uint8_t WrelayPins[maxW][2] = {{D1, D2}, {D5, D6}, {0, 0}, {0, 0}};
uint8_t WinputPins[maxW][2] = {{D3, D4}, {D7, D8}, {0, 0}, {0, 0}};
uint8_t WextInPins[maxW][2] = {{D5, D6}, {255, 255}, {255, 255}, {255, 255}};

char winGroupTopics[4][30];
char winTopics[maxW][40];

void updateTopics()
{
    strcpy(winGroupTopics[0], "myHome/Windows");
    strcpy(winGroupTopics[1], "myHome/lockdown");
    strcpy(winGroupTopics[2], "myHome/alarmMonitor");
    strcpy(winGroupTopics[3], "myHome/Windows/gFloor");

    strcpy(winTopics[0], "myHome/Windows/gFloor/Win1");
    strcpy(winTopics[1], "myHome/Windows/gFloor/Win2");
    strcpy(winTopics[2], "myHome/Windows/gFloor/Win3");
    strcpy(winTopics[3], "myHome/Windows/gFloor/Win4");
}
/*************************************************************************/