#define RELAY_ON HIGH
#define STATES_TXT (char *[]) { "Stop", "Up", "Down", "Err" }
#define REASONS_TXT (char *[]) { "Timeout", "Button", "Button2", "Lockdown" }

struct MSGstr
{
    uint8_t state;     /* Up/Down/ Off */
    uint8_t reason; /* What triggered the button */
};
enum REASONS : const uint8_t
{
    TIMEOUT,
    BUTTON,
    BUTTON2,
    LCKDOWN,
};
enum WIN_STATES : const uint8_t
{
    STOP,
    UP,
    DOWN,
    ERR
};
