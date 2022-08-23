#define MAX_SW 4
#define DEBUG_MODE true
#define MAX_TOPIC_SIZE 40                   // <----- Verfy max Topic size
#define JSON_FILESIZE 600 * MAX_SW          // <----- Verfy Json size

#define DEFAULT_LOG_TOPIC "myHome/log"      // CASE of File read error 
#define DEFAULT_MSG_TOPIC "myHome/Messages" // CASE of File read error from flash (topics)

const char *INPUT_ORG[5] = {"Timeout", "Button", "MQTT", "PowerON", "Resume Reboot"};
char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"}; // <----- Verfy file names

bool oab[MAX_SW];
bool powr[MAX_SW];
char SwNames[MAX_SW][12];
uint8_t NUM_SWITCHES;
