#define VER "ESP8266_0.4"
#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 200
#define MQTT_OFFSET 10
#define QUERY_OFFSET 40

enum sys_states :byte
{
  WIN_STOP,
  WIN_UP,
  WIN_DOWN,
  WIN_ERR,
  QUERY,
  RESET,
  BOOT
};
 const char *winstate[] = {"Off", "Up", "Down"};

byte currentRelay_state = 0;
const byte delay_loop = 100;    //ms

bool useAutoOff = false;
int autoOff_time = 300;        //seconds
unsigned long autoOff_clk = 0;
