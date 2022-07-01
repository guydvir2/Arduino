#define MAX_NUM_SCHEDS 2
#define FlashParameters 0 // <----- Code Parameters or Flash

myIOT2 iot;

Chrono chron_0(Chrono::SECONDS);
Chrono chron_1(Chrono::SECONDS);
Chrono *chronVector[MAX_NUM_SCHEDS] = {&chron_0, &chron_1};

Button2 button_0;
Button2 button_1;
Button2 *buttonVector[MAX_NUM_SCHEDS] = {&button_0, &button_1};

TurnLightsON light_0;
TurnLightsON light_1;
TurnLightsON *lightVector[MAX_NUM_SCHEDS] = {&light_0, &light_1};

enum TRIG_SRC : const uint8_t
{
  BTN,
  TIMEOUT,
  MQTT_CMD,
  PWR_ON,
};
enum SWITCH_TYPES : const uint8_t
{
  MOMENTARY,
  ON_OFF,
  TRIGGER_SERNSOR,
  BTN_TO_PWM,
  MULTI_PRESS
};

const char *VER = "Scheduler_v0.1";
char *sketch_paramfile = "/sketch_param.json";
const char *SRCS[] = {"Button", "Timeout", "MQTT", "PowerOn"};

/* ~~~~~~~~~~~~~~~~~~~~~~ Values get updated from parameter file ~~~~~~~~~~~~~~~~~~ */
uint8_t numSW = 1;            /* Num of switches: 1 or 2 */
int PWM_res = 1023;           /* Not change ESP8266 */
int sketch_JSON_Psize = 1350; /* Pass JSON size for Flash Parameter*/

bool useInputs[MAX_NUM_SCHEDS] = {true, true};
bool useIndicLED[MAX_NUM_SCHEDS] = {true, false}; /* use indication leds when ON*/

bool indic_ON[MAX_NUM_SCHEDS] = {HIGH, false};
bool output_ON[MAX_NUM_SCHEDS] = {HIGH, HIGH};   /* OUTPUT when ON is HIGH or LOW */
bool inputPressed[MAX_NUM_SCHEDS] = {LOW, LOW}; /* High or LOW on button press */
bool OnatBoot[MAX_NUM_SCHEDS] = {false, false}; /* After reboot- On or Off */
bool outputPWM[MAX_NUM_SCHEDS] = {true, true};
bool dimPWM[MAX_NUM_SCHEDS] = {true, true};

uint8_t trigType[MAX_NUM_SCHEDS] = {0, 0};          /* Input type */
uint8_t inputPin[MAX_NUM_SCHEDS] = {D6, D5};        /* IO for inputs */
uint8_t outputPin[MAX_NUM_SCHEDS] = {D1, D2};         /* IO for outputs */
uint8_t indicPin[MAX_NUM_SCHEDS] = {D2, 2};          /* IO for idication LEDS */
uint8_t defPWM[MAX_NUM_SCHEDS] = {2, 2};            /* Default PWM value for some cases not specified */
uint8_t limitPWM[MAX_NUM_SCHEDS] = {80, 80};        /* Limit total intensity, 1-100 */
uint8_t maxPWMpresses[MAX_NUM_SCHEDS] = {3, 3};     /* Limit total intensity, 1-100 */
char sw_names[MAX_NUM_SCHEDS][20] = {"Bulb", "LED"}; /* Name of each Switch, as shown on MQTT msg */

int maxTimeout[MAX_NUM_SCHEDS] = {600, 600};
int defaultTimeout[MAX_NUM_SCHEDS] = {10, 20};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int lastPWMvalue[] = {0, 0};
int timeouts[MAX_NUM_SCHEDS] = {0, 0};

void notifyAdd(uint8_t &i, int &_add, const char *trigger);
void notifyOFF(uint8_t &i, int &_elapsed, const char *trigger);
void notifyON(uint8_t &i, const char *trigger);
void ONcmd(uint8_t i, uint8_t _TO = 0, const char *trigger = nullptr, uint8_t _PWMstep = 0);
void OFFcmd(uint8_t i, const char *trigger = nullptr);