#define TOT_Relays 8
#define TOT_Inputs 12
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

#define OUTPUT_ON HIGH
#define BUTTON_PRESSED LOW
#define HWturnON(i) digitalWrite(i, OUTPUT_ON)
#define HWturnOFF(i) digitalWrite(i, !OUTPUT_ON)

struct SwitchStruct
{
  uint8_t id = 0;
  uint8_t type = 0;   /* <0> is ON-Off Switch, <1> push button */
  uint8_t RFch = 255; /* Define RF channel. leave 255 to disable */
  uint8_t outPin = 255;
  bool virtCMD = false;    /* Trigger a MQTT or a Relay */
  bool useButton = true;   /* Input is an option */
  bool guessState = false; /* when using IOcmd and a pushbutton state can not be known */
  char Topic[MAX_TOPIC_SIZE] = "EMPTY";

  Button2 button; /* inputs only */
};

void print_sw_struct(SwitchStruct &sw)
{
  Serial.println("<<<<<<<<<<<< Switch Entity >>>>>>>>>>>>>>");

  Serial.print("ID:\t");
  Serial.println(sw.id);
  Serial.print("RFch:\t");
  Serial.println(sw.RFch);
  Serial.print("inPin:\t");
  Serial.println(sw.button.getPin());
  Serial.print("outPin:\t");
  Serial.println(sw.outPin);
  Serial.print("type:\t");
  Serial.println(sw.type);
  Serial.print("virtCMD:\t");
  Serial.println(sw.virtCMD);
  Serial.print("useButton:\t");
  Serial.println(sw.useButton);
  Serial.print("guessState:\t");
  Serial.println(sw.guessState);
  Serial.print("Topic:\t");
  Serial.println(sw.Topic);
}
WinSW *winSW_V[TOT_Relays / 2] = {nullptr, nullptr, nullptr, nullptr};
SwitchStruct *SW_v[TOT_Inputs] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

char *buttMQTTcmds[2] = {"on", "off"};
char *turnTypes[] = {"MQTT", "Button", "Remote"};

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
uint8_t ParameterFile_preset=1;

int RF_keyboardCode[] = {3135496, 3135492, 3135490, 3135489};

char winGroupTopics[2][MAX_TOPIC_SIZE];
char buttGroupTopics[3][MAX_TOPIC_SIZE];

#if defined(ESP32)
uint8_t buttonPins[TOT_Relays] = {19, 17, 16, 5, 0, 0, 0, 0};
uint8_t relayPins[TOT_Relays] = {25, 26, 33, 32, 0, 0, 0, 0};
uint8_t WrelayPins[TOT_Relays / 2][2] = {{33, 32}, {25, 26}, {0, 0}, {0, 0}};
uint8_t WinputPins[TOT_Relays / 2][2] = {{16, 5}, {19, 17}, {0, 0}, {0, 0}};
uint8_t WextInPins[TOT_Relays / 2][2] = {{5, 4}, {255, 255}, {255, 255}, {255, 255}};
#elif defined(ESP8266)
uint8_t buttonPins[TOT_Relays] = {D7, D2, D3, D4, 0, 0, 0, 0};
uint8_t relayPins[TOT_Relays] = {D8, D6, D7, D8, 0, 0, 0, 0};
uint8_t WrelayPins[TOT_Relays / 2][2] = {{D3, D4}, {D5, D6}, {0, 0}, {0, 0}};
uint8_t WinputPins[TOT_Relays / 2][2] = {{D5, D6}, {D7, D8}, {0, 0}, {0, 0}};
uint8_t WextInPins[TOT_Relays / 2][2] = {{D7, D8}, {255, 255}, {255, 255}, {255, 255}};
#endif
// <<<<<<<<<<<<<<<<<<<<<  WinSW Paramters >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* Entity types: 0 is window, 1 is SW */
/* virtCMD: is entity calls a MQTT cb, or actually switching a gpio */
/* SW_buttonTypes : 0 is no input, 1 is ONOFFsw, 2 is a push button */
/* WextInputs - is a window entity need additional input */