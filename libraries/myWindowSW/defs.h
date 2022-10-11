#pragma once

// #define STATES_TXT \
//     (char *[]) { "Off", "Up", "Down", "Err" }
// #define REASONS_TXT \
//     (char *[]) { "Timeout", "Button", "Button2", "Lockdown", "MQTT" }

struct Win_act_telem
{
    uint8_t state;  /* Up/Down/ Off */
    uint8_t reason; /* What triggered the button */
    bool lockdown_state = false;
};
struct Win_props
{
    uint8_t id;
    uint8_t inpins[2];
    uint8_t outpins[2];
    uint8_t inpins2[2];

    bool lockdown = false;
    bool extSW = false;
    bool virtCMD = false;
    char *name;
};

enum REASONS : const uint8_t
{
    TIMEOUT,
    BUTTON,
    BUTTON2,
    LCKDOWN,
    MQTT
};
enum WIN_STATES : const uint8_t
{
    STOP,
    UP,
    DOWN,
    ERR
};