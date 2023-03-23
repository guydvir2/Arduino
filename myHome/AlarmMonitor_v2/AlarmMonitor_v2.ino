
/*
   ESP8266 Alarm monitoring system, by guyDvir Apr.2019

   Pupose of this code is to monitor, notify and change syste, states of an existing
   Alarm system ( currently using PIMA alarm system incl. RF detectors) has the
   following hardware:
   1) Main Controller AKA " the brain"
   2) KeyPad for entering user codes and change syste states.
   3) "the brain" has relay inputs knows as external key operation:
 */

// UPDATE 15/06/2020 - removeing Telegram Nitofocation from code. update iot - 7.11
// Previous version 2.1, iot1.43

/*Update 11/2020
1.Update iot Services
2. know bug when arm_home using code, it arm_away.
*/

/*Update 5/2021
1.Update iot2 Services
2. know bug when arm_home using code, it arm_away.
*/

/*Update 7/2021
1.Update iot2 Services
2. MQTT help2 fix
3. know bug when arm_home using code, it arm_away.
*/

/*Update 8/2021
1.Update iot2 Services
2. switch 2 uint_8 instead of byte
3. know bug when arm_home using code, it arm_away.
*/

/*Update 12/2021
1. Update iot2 Services. change network
2. change to cleaner code. err 3 still persists
3. know bug when arm_home using code, it arm_away.
*/

/*Update 04/2022
1. Fixing main issue of armed_home and armed_away.
2. swap D5 and D6 - Zone8 is now responsible for armed_home and KEY for armed_away.
3. version 4
*/
/*

                +==========+=============+============+============+===============+
                |  INPUT1  |   INPUT2    |   OUTPUT1  |  OUTPUT2   |      Sys      |
                | Arm Indic| Alarm indic | Home Relay | Away Relay |     State     |
                +==========+=============+============+============+===============+
                |    1     |      1      |      0     |     0      | disarmed All  |
                +----------+-------------+------------+------------+---------------+
                |    0     |      1      |      1     |     0      | Home Arm code |
                +----------+-------------+------------+------------+---------------+
                |    0     |      1      |      0     |     1      | Away Arm code |
                +----------+-------------+------------+------------+---------------+
                |    0     |      1      |      0     |     0      |Home/awa keypad|
                +----------+-------------+------------+------------+---------------+
                |    0     |      0      |      0/1   |     0/1    |     Alarm     |
                +----------+-------------+------------+------------+---------------+


                +==============+=============+=============+
                |  System      |     GPIO    |     I/O     |
                |  State       |   ESP8266   |     PIMA    |
                +==============+=============+=============+
                | Alarm IND    | D4 PULL_UP  |     ALARM   |
                +--------------+-------------+-------------+
                | Armed IND    | D3 PULL_UP  |    ON/OFF   |
                +--------------+-------------+-------------+
                | Arm-Home CMD |      D5     |     KEY     |
                +--------------+-------------+-------------+
                | Arm-Away CMD |      D6     |     Z8      |
                +--------------+-------------+-------------+


                              ------ > PIMA SIDE <-------
                +---------------------------------------------------+
                |   *       *       *       *           *     *     |
                |  ARM    ALARM    12V     GND         KEY   Z8     |
                |  [INDICATIONS]            |         [COMMANDS]    |
                |                           |                       |
                |   ###    ###              |          ###   ###    |
                |   ###    ###              |          ###   ###    |
                |   12v    12V              |          3v    3v     |
                |   FET    FET              |          FET   FET    |
                |                           |                       |
                |                           +------------------+    |
                |   [INPUTS]                     [OUTPUTS]     |    |
                |   *       *                    *      *      *    |
                |  D3      D4                   D6     D5      GND  |
                +----------------------------------------------------
                               ------ > ESP SIDE <-------




*/
#include <Arduino.h>

// ~~~~ HW Pins and States ~~~~
#define DELAY_TO_REACT 3000 // millis

#define STATE_ON HIGH
#define SYSTEM_STATE_ON LOW
#define SYSTEM_STATE_ARM_PIN D3      //  Indication system is Armed
#define SYSTEM_STATE_ALARM_PIN D4    //  Indication system is Alarmed
#define SET_SYSTEM_ARMED_HOME_PIN D5 //   (Set system)  armed_Home
#define SET_SYSTEM_ARMED_AWAY_PIN D6 //   (Set system)  Armed_Away

const char *VER = "alarmMon_4.0";
const char *sys_states[5] = {
    "armed_home",
    "armed_away",
    "disarmed",
    "pending",
    "triggered"};
enum sys_state : const uint8_t
{
        ARMED_HOME_CODE,
        ARMED_AWAY_CODE,
        DISARMED,
        ARMED_KEYPAD,
        ALARMING,
        ARM_ERR,
};

bool indication_ARMED_lastState = false;
bool indication_ALARMED_lastState = false;

#include <myIOT2.h>
#include "myIOT_settings.h"

void startGPIOs()
{
        pinMode(SET_SYSTEM_ARMED_HOME_PIN, OUTPUT);    /* Pin to switch system state*/
        pinMode(SET_SYSTEM_ARMED_AWAY_PIN, OUTPUT);    /* Pin to switch system state*/
        pinMode(SYSTEM_STATE_ARM_PIN, INPUT_PULLUP);   /* Pin to get system state*/
        pinMode(SYSTEM_STATE_ALARM_PIN, INPUT_PULLUP); /* Pin to get system state*/

        _detect_state_change(SYSTEM_STATE_ARM_PIN, indication_ARMED_lastState);
        _detect_state_change(SYSTEM_STATE_ALARM_PIN, indication_ALARMED_lastState);

        pub_systemState(get_systemState()); /* in case of reset - update State */
}
void disarm()
{
        digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, !STATE_ON);
        digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, !STATE_ON);
        delay(DELAY_TO_REACT); // time to system to react.

        if (get_systemState() != DISARMED)
        {
                iot.pub_log("[System]:failed to disarm. Reset sent");
                iot.sendReset("OFF_FAIL");
        }
}
void armHome()
{
        digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, STATE_ON);
}
void armAway()
{
        digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, STATE_ON);
}
uint8_t get_systemState()
{
        /* Alarm can be in one of three state:
        Alarming, Armed, disarmed
         */
        if (digitalRead(SYSTEM_STATE_ALARM_PIN) == SYSTEM_STATE_ON) /* Check if alarm pin is ON */
        {
                return ALARMING;
        }
        else if (digitalRead(SYSTEM_STATE_ARM_PIN) == SYSTEM_STATE_ON) /* Check if Armed pin is ON */
        {
                if (digitalRead(SET_SYSTEM_ARMED_HOME_PIN) == STATE_ON && digitalRead(SET_SYSTEM_ARMED_AWAY_PIN) == !STATE_ON) /* set by code TO home_Armed ?*/
                {
                        return ARMED_HOME_CODE;
                }
                else if (digitalRead(SET_SYSTEM_ARMED_AWAY_PIN) == STATE_ON && digitalRead(SET_SYSTEM_ARMED_HOME_PIN) == !STATE_ON) /* set by code TO away_Armed ?*/
                {
                        return ARMED_AWAY_CODE;
                }
                else if (digitalRead(SET_SYSTEM_ARMED_AWAY_PIN) == STATE_ON && digitalRead(SET_SYSTEM_ARMED_HOME_PIN) == STATE_ON)
                {
                        return ARM_ERR;
                }
                else
                {
                        return ARMED_KEYPAD; /* armed, but without ext inputs, so it is keypad */
                }
        }
        else
        {
                return DISARMED;
        }
}
void set_armState(uint8_t req_state)
{
        char a[50];
        uint8_t curState = get_systemState();
        if (req_state != curState)
        {
                if (req_state == DISARMED) /* disarm when armed using keybaord */
                {
                        if (curState == ARMED_KEYPAD)
                        {
                                /* Armed by keypad. Can't disarm without any of setter set to arm.
                                Have to fake an input by system in order to turn off */
                                armHome();
                                delay(DELAY_TO_REACT); // Time for system to react to fake state change
                        }
                        disarm();
                        sprintf(a, "System change: [disarmed] using [Code]");
                        iot.pub_msg(a);
                }
                else if (req_state == ARMED_HOME_CODE)
                {
                        if (curState != DISARMED)
                        {
                                disarm();
                        }
                        armHome();
                }
                else if (req_state == ARMED_AWAY_CODE)
                {
                        if (curState != DISARMED)
                        {
                                disarm();
                        }
                        armAway();
                }
        }
        else
        {
                sprintf(a, "System No-change: already in [%s] state", sys_states[curState]);
                iot.pub_msg(a);
        }
}

bool _detect_state_change(uint8_t _pin, bool &_lastState)
{
        const uint8_t deBounceInt = 50;
        if (digitalRead(_pin) != _lastState)
        {
                delay(deBounceInt);
                if (digitalRead(_pin) != _lastState)
                {
                        delay(DELAY_TO_REACT);
                        _lastState = digitalRead(_pin);
                        return true;
                }
                else
                {
                        return false;
                }
        }
        else
        {
                return false;
        }
}
void check_systemState_armed()
{
        if (_detect_state_change(SYSTEM_STATE_ARM_PIN, indication_ARMED_lastState))
        {
                char a[30];
                uint8_t curState = get_systemState();
                pub_systemState(curState);
                sprintf(a, "System state: [%s]", sys_states[curState]);
                iot.pub_msg(a);

                if (curState == DISARMED) /* Verify all inputs are OFF */
                {
                        disarm();
                }
                else if (curState == ARMED_KEYPAD)
                {
                        sprintf(a, "Keypad: [Armed]");
                        iot.pub_msg(a);
                }
                else if (curState == ARMED_HOME_CODE || ARMED_AWAY_CODE)
                {
                        sprintf(a, "Code: [Armed]");
                        iot.pub_msg(a);
                }
        }
}
void check_systemState_alarming()
{
        if (_detect_state_change(SYSTEM_STATE_ALARM_PIN, indication_ALARMED_lastState))
        {
                uint8_t curState = get_systemState();
                pub_systemState(curState);

                if (curState == ALARMING)
                {
                        iot.pub_msg("[Alarm] is triggered");
                }
                else
                {
                        iot.pub_msg("[Alarm] stopped");
                }
        }
}

void setup()
{
        startIOTservices();
        startGPIOs();
}
void loop()
{
        iot.looper();
        check_systemState_armed();
        check_systemState_alarming();
}
