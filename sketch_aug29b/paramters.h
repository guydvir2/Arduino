#define TOT_Relays 8
#define TOT_Inputs 12
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size


const char *SW_MQTT_cmds[] = {"off", "on"};
const char *winMQTTcmds[3] = {"off", "up", "down"};
const char *SW_Types[] = {"Button", "Timeout", "MQTT", "Remote"};
const char *EntTypes[2] = {"win", "sw"}; /* Prefix to address client types when using MQTT */

uint8_t lastUsed_inIO = 0;
uint8_t lastUsed_outIO = 0;
uint8_t swEntityCounter = 0;
uint8_t winEntityCounter = 0;

uint8_t ParameterFile_preset = 3;

WinSW *winSW_V[TOT_Relays / 2] = {nullptr, nullptr, nullptr, nullptr};
smartSwitch *SW_v[TOT_Inputs] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

/* ****** Paramters that will be updated from FLASH saved on file ******* */
uint8_t RFpin = 27;
uint8_t linkRF2SW[4] = {255, 255, 255, 255};

int RF_keyboardCode[] = {3135496, 3135492, 3135490, 3135489};
uint8_t outPinsArray[TOT_Relays] = {255, 255, 255, 255, 255, 255, 255, 255};
uint8_t inPinsArray[TOT_Inputs] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

char winGroupTopics[2][MAX_TOPIC_SIZE];
char buttGroupTopics[3][MAX_TOPIC_SIZE];

// <<<<<<<<<<<<<<<<<<<<<  WinSW Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>