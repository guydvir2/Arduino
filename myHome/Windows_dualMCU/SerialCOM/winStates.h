#define MQTT_OFFSET 10
#define QUERY_OFFSET 40
#define LOOP_DELAY 10
#define QUERY_INIT 100

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