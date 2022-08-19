#define numSW 4
#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW

#define turnON(i) digitalWrite(i, OUTPUT_ON)
#define turnOFF(i) digitalWrite(i, !OUTPUT_ON)
#define isON(i) digitalRead(i) == OUTPUT_ON 
#define isPRESSED(i) digitalRead(i) == BUTTON_PRESSED

const uint8_t RFpin = 27;
#if defined(ESP32)
const uint8_t buttonPins[] = {19, 17, 16, 5};
const uint8_t relayPins[] = {25, 26, 33, 32};
#elif defined(ESP8266)
const uint8_t buttonPins[] = {D1, D2, D3, D4};
const uint8_t relayPins[] = {D5, D6, D7, D8};
#endif

const bool buttonTypes[] = {0, 0, 0, 1}; /* <0> is ON-Off Switch, <1> push button */
const int KB_codes[] = {3135496, 3135492, 3135490, 3135489};
const char *ButtonNames[] = {"Saloon", "Entrance", "Parking", "Kitchen"};
const char *turnTypes[] = {"MQTT", "Button", "RemoteControl", "Code"};

enum OPerTypes : const uint8_t
{
    MQTT,
    BUTTON,
    RF,
    BIT
};