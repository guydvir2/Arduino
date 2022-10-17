#ifndef _homeCTRL_H
#define _homeCTRL_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <myWindowSW.h>  /* WinSW Entities */
#include <smartSwitch.h> /* smartSwitch Entities */

struct Cotroller_Ent_telemetry
{
    uint8_t id;    /* of entity instance*/
    uint8_t type;  /* Entiry type 0- win. 1 sw */
    uint8_t state; /* Up/Down/ Off */
    uint8_t trig;  /* What triggered the button */

    bool newMSG = false;
    unsigned long timeout = 0;
};
enum ENT_TYPE : const uint8_t
{
    WIN_ENT,
    SW_ENT
};

class homeCtl
{
#define TOT_Relays 8
#define TOT_Inputs 8
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

private:
    bool _use_RF = false;
    uint8_t _RFpin = 27;
    uint8_t _inIOCounter = 0;
    uint8_t _outIOCounter = 0;
    uint8_t _swEntityCounter = 0;
    uint8_t _winEntityCounter = 0;
    uint8_t _RF_ch_2_SW[4] = {255, 255, 255, 255};
    int _RF_freq[4] = {3135496, 3135492, 3135490, 3135489};

public:
    const char *ver = "smartController_v0.1";

    char *SW_MQTT_cmds[2] = {"off", "on"};
    char *winMQTTcmds[3] = {"off", "up", "down"};
    char *WinStates[4] = {"Off", "Up", "Down", "Err"};
    char *WinTrigs[5] = {"Timeout", "Button", "Button2", "Lockdown", "MQTT"};
    char *SW_Types[4] = {"Button", "Timeout", "MQTT", "Remote"};
    char *EntTypes[2] = {"win", "sw"}; /* Prefix to address client types when using MQTT */

private:
    Cotroller_Ent_telemetry _MSG;
    RCSwitch *RF_v = nullptr;
    WinSW *winSW_V[TOT_Relays / 2] = {nullptr, nullptr, nullptr, nullptr};
    smartSwitch *SW_v[TOT_Inputs] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    void _init_RF(uint8_t i);
    void _toggle_SW_RF(uint8_t i);

    void _SW_newMSG(uint8_t i);
    void _Win_newMSG(uint8_t i);

    void _RF_loop();
    void _SW_loop();
    void _Win_loop();

public:
    homeCtl();
    bool loop();
    void set_RF(uint8_t pin = 255);             /* IO that RF recv is connected to */
    void set_RFch(int arr[], uint8_t arr_size); /* Radio freq to listen to. belong to a remote control */
    void set_ent_name(uint8_t i, uint8_t ent_type, const char* name);
    void Win_init_lockdown();
    void Win_release_lockdown();

    uint8_t get_ent_counter(uint8_t type);
    uint8_t get_ent_state(uint8_t type, uint8_t i);
    char *get_ent_name(uint8_t i, uint8_t ent_type);
    char *get_ent_ver(uint8_t type);
    void get_telemetry(Cotroller_Ent_telemetry &M);
    void get_entity_prop(uint8_t ent_type, uint8_t i, SW_props &sw_prop);
    void get_entity_prop(uint8_t ent_type, uint8_t i, Win_props &win_prop);

    void create_Win(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, bool is_virtual = false, bool use_ext_sw = false);
    void create_SW(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, uint8_t sw_type, bool is_virtual = false, int timeout_m = 1, uint8_t RF_ch = 255);

    void Win_switchCB(uint8_t i, uint8_t state);
    void SW_switchCB(uint8_t i, uint8_t state, unsigned int TO = 0);

    void clear_telemetryMSG();
};

#endif
