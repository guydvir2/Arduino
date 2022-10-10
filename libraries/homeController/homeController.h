#ifndef _homeCTRL_H
#define _homeCTRL_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <myWindowSW.h>  /* WinSW Entities */
#include <smartSwitch.h> /* smartSwitch Entities */

struct Ctl_MSGstr
{
    bool virtCMD;
    bool newMSG = false;

    uint8_t id;
    uint8_t type;   /* Entiry type 0- win. 1 sw */
    uint8_t state;  /* Up/Down/ Off */
    uint8_t reason; /* What triggered the button */
};

class homeCtl
{
#define TOT_Relays 8
#define TOT_Inputs 12
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

private:
    bool _use_RF = false;
    uint8_t _RFpin = 27;
    uint8_t _inIOCounter = 0;
    uint8_t _outIOCounter = 0;
    uint8_t _swEntityCounter = 0;
    uint8_t _winEntityCounter = 0;
    uint8_t _input_pins[TOT_Inputs];
    uint8_t _output_pins[TOT_Relays];
    uint8_t _RF_ch_2_SW[4] = {255, 255, 255, 255};
    int _RF_freq[4] = {3135496, 3135492, 3135490, 3135489};

    const char *ver = "smartController_v0.1";

    /* ±±±±±±±±± Filenames and directories for each controller ±±±±±±±±±±± */
    char parameterFiles[4][30];
    // const char *dirs[] = {"Fail", "Cont_A", "Cont_B", "Cont_C", "Cont_D", "Cont_test"};
    // const char *FileNames_common[2] = {"myIOT_param.json", "Hardware.json"};
    // const char *FileNames_dedicated[2] = {"myIOT2_topics.json", "sketch_param.json"};
    /* ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±± */

public:
    char *SW_MQTT_cmds[2] = {"off", "on"};
    char *winMQTTcmds[3] = {"off", "up", "down"};
    char *SW_Types[4] = {"Button", "Timeout", "MQTT", "Remote"};
    char *EntTypes[2] = {"win", "sw"}; /* Prefix to address client types when using MQTT */

private:
    Ctl_MSGstr _MSG;
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
    void set_inputs(uint8_t arr[], uint8_t arr_size);
    void set_outputs(uint8_t arr[], uint8_t arr_size);
    void set_RFch(uint8_t arr[], uint8_t arr_size);
    uint8_t get_SW_entCounter();
    uint8_t get_Win_entCounter();
    uint8_t get_inputPins(uint8_t i);
    uint8_t get_outputPins(uint8_t i);
    auto get_win_property(WinSW &win);
    void get_Win_name(uint8_t i, char &name);
    void get_SW_name(uint8_t i, char &name);

    uint8_t get_Win_state(uint8_t i);
    uint8_t get_SW_state(uint8_t i);
    bool SW_use_timeout(uint8_t);

    void create_Win(char *topic, bool is_virtual = false, bool use_ext_sw = false);
    void create_SW(char *topic, uint8_t sw_type, bool is_virtual = false, int timeout_m = 1, uint8_t RF_ch = 255);

    void Win_switchCB(uint8_t i, uint8_t state);
    void SW_switchCB(uint8_t i, uint8_t state, unsigned int TO = 0);

    void clear_telemetryMSG();
    void get_telemetry(Ctl_MSGstr &M);
};

#endif
