#define TOT_Relays 8
#define TOT_Inputs 12
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size


// void print_sw_struct(SwitchStruct &sw)
// {
//   Serial.println("<<<<<<<<<<<< Switch Entity >>>>>>>>>>>>>>");

//   Serial.print("ID:\t");
//   Serial.println(sw.id);
//   Serial.print("RFch:\t");
//   Serial.println(sw.RFch);
//   Serial.print("inPin:\t");
//   Serial.println(sw.button.getPin());
//   Serial.print("outPin:\t");
//   Serial.println(sw.outPin);
//   Serial.print("type:\t");
//   Serial.println(sw.type);
//   Serial.print("virtCMD:\t");
//   Serial.println(sw.virtCMD);
//   Serial.print("useButton:\t");
//   Serial.println(sw.useButton);
//   Serial.print("guessState:\t");
//   Serial.println(sw.guessState);
//   Serial.print("Topic:\t");
//   Serial.println(sw.Topic);
// }
WinSW *winSW_V[TOT_Relays / 2] = {nullptr, nullptr, nullptr, nullptr};
smartSwitch *SW_v[TOT_Inputs]={nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
// SwitchStruct *SW_v[TOT_Inputs] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

char *buttMQTTcmds[2] = {"on", "off"};
char *turnTypes[] = {"Button", "MQTT", "Remote"};

const char *EntTypes[2] = {"win", "sw"}; /* Prefix to address client types when using MQTT */
const char *winMQTTcmds[3] = {"off", "up", "down"};

enum OPerTypes : const uint8_t
{
  _MQTT,
  _BUTTON,
  RF
};
uint8_t lastUsed_inIO = 0;
uint8_t lastUsed_outIO = 0;
uint8_t swEntityCounter = 0;
uint8_t winEntityCounter = 0;

/* ****** Paramters that will be updated from FLASH saved on file ******* */
uint8_t RFpin = 27;
uint8_t ParameterFile_preset = 3;

int RF_keyboardCode[] = {3135496, 3135492, 3135490, 3135489};
uint8_t outPinsArray[TOT_Relays] = {255, 255, 255, 255, 255, 255, 255, 255};
uint8_t inPinsArray[TOT_Inputs] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

char winGroupTopics[2][MAX_TOPIC_SIZE];
char buttGroupTopics[3][MAX_TOPIC_SIZE];

// <<<<<<<<<<<<<<<<<<<<<  WinSW Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* Entity types: 0 is window, 1 is SW */
/* virtCMD: is entity calls a MQTT cb, or actually switching a gpio */
/* SW_buttonTypes : 0 is no input, 1 is ONOFFsw, 2 is a push button */
/* WextInputs - is a window entity need additional input */