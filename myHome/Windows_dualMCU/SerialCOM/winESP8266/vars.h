#define VER "ESP8266_0.5"
#define JSON_SIZE_IOT 400
#define JSON_SIZE_SKETCH 200

const char *winstate[] = {"Off", "Up", "Down"};

byte currentRelay_state = 0;
const byte delay_loop = 10*LOOP_DELAY;

bool useAutoOff = false;
int autoOff_time = 300;        //seconds
unsigned long autoOff_clk = 0;
