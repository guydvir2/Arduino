#define MAX_Relays 8
#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW

#define turnON(i) digitalWrite(i, OUTPUT_ON)
#define turnOFF(i) digitalWrite(i, !OUTPUT_ON)

#if defined(ESP32)
const uint8_t buttonPins[MAX_Relays] = {19, 17, 16, 5, 0, 0, 0, 0};
const uint8_t relayPins[MAX_Relays] = {25, 26, 33, 32, 0, 0, 0, 0};
#elif defined(ESP8266)
const uint8_t buttonPins[MAX_Relays] = {D1, D2, D3, D4, 0, 0, 0, 0};
const uint8_t relayPins[MAX_Relays] = {D5, D6, D7, D8, 0, 0, 0, 0};
#endif

bool useRF = true;
bool useButton = true;
uint8_t numSW = 4;
uint8_t RFpin = 27;

const bool buttonTypes[] = {0, 0, 0, 1}; /* <0> is ON-Off Switch, <1> push button */
const int KB_codes[] = {3135496, 3135492, 3135490, 3135489};
const char *ButtonNames[] = {"Saloon", "Entrance", "Parking", "Kitchen"};
const char *turnTypes[] = {"MQTT", "Button", "Remote", "Code"};

enum OPerTypes : const uint8_t
{
    MQTT,
    BUTTON,
    RF,
    BIT
};