// ~~~~ HW Pins and Statdes ~~~~
#define RELAY1 D5
#define INPUT1 D6
#define indic_LEDpin D7

// ~~~~~~~~ state Vars ~~~~~~~~~~~
#define RelayOn HIGH
#define SwitchOn LOW
#define ledON HIGH

// ~~~~~~~TimeOut Constants ~~~~~~
#define maxTO 150         //minutes to timeout even in ON state
#define timeIncrements 15 //minutes each button press
#define time_NOref_OLED 5 // seconds to stop refresh OLED

// ~~~~~~~~~~~~ OLED ~~~~~~~~~~~~~~~~~~~
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 // double screen size
#define OLED_RESET LED_BUILTIN

// ********** Sketch Services  ***********
#define VEr "theDude_v3.1"


// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "WaterBoiler"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP ""
