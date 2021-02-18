#define VER "ESP8266_0.1"
#define RelayOn LOW
#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 200
#define MQTT_OFFSET 10
#define QUERY_OFFSET 40

// enum sys_states
// {
//   WIN_STOP,
//   WIN_UP,
//   WIN_DOWN,
//   WIN_ERR,
//   QUERY,
//   RESET,
//   BOOT
// };

const byte WIN_STOP = 0;
const byte WIN_UP = 1;
const byte WIN_DOWN = 2;
const byte WIN_ERR = 3;
const byte QUERY = 4;
const byte RESET = 5;
const byte BOOT = 6;

byte currentRelay_state = 0;

const byte delay_switch = 100;        //ms 10 times more than MCU's looper - to have enough time to react to change
const byte delay_loop = 200;          //ms
const byte delay_resend_serial = 100; //ms to send again serial message

bool useAutoOff = false;
byte autoOff_time = 300; //seconds
unsigned long autoOff_clk = 0;
