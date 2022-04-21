
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

/*
/*Update 12/2021
1. Update iot2 Services. change network
2. change to cleaner code. err 3 still persists
3. know bug when arm_home using code, it arm_away.
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
                | Arm-Home CMD |      D6     |     KEY     |
                +--------------+-------------+-------------+
                | Arm-Away CMD |      D5     |     Z8      |
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
#define STATE_ON HIGH
#define SYSTEM_STATE_ON LOW
#define SYSTEM_STATE_ARM_PIN D3      //  Indication system is Armed
#define SYSTEM_STATE_ALARM_PIN D4    //  Indication system is Alarmed
#define SET_SYSTEM_ARMED_HOME_PIN D6 //   (Set system)  armed_Home
#define SET_SYSTEM_ARMED_AWAY_PIN D5 //   (Set system)  Armed_Away

#define VER "alarmMon_4.0"
char *sys_states[] = {"armed_home", "armed_away", "disarmed", "pending", "triggered"};
enum sys_state : const uint8_t
{
        ARMED_HOME_CODE,
        ARMED_AWAY_CODE,
        DISARMED,
        ARMED_KEYPAD,
        ALARMING,
};
const uint8_t systemPause = 2; // seconds, delay to system react
bool indication_ARMED_lastState = false;
bool indication_ALARMED_lastState = false;

#include <myIOT2.h>
#include "myIOT_settings.h"

void startGPIOs()
{
        pinMode(SET_SYSTEM_ARMED_HOME_PIN, OUTPUT);
        pinMode(SET_SYSTEM_ARMED_AWAY_PIN, OUTPUT);
        pinMode(SYSTEM_STATE_ARM_PIN, INPUT_PULLUP);
        pinMode(SYSTEM_STATE_ALARM_PIN, INPUT_PULLUP);

        _detect_state_change(SYSTEM_STATE_ARM_PIN, indication_ARMED_lastState);
        _detect_state_change(SYSTEM_STATE_ALARM_PIN, indication_ALARMED_lastState);
}
void allOff()
{
        uint8_t retries = 0;
        const uint8_t max_retries = 3;
        while (digitalRead(SYSTEM_STATE_ARM_PIN) == SYSTEM_STATE_ON && retries < max_retries)
        {
                digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, !STATE_ON);
                digitalWrite(SET_SYSTEM_ARMED_AWAY_PIN, !STATE_ON);
                delay(systemPause * 1000);
                retries++;
        }

        if (retries == max_retries || get_systemState() != DISARMED)
        {
                iot.pub_log("[System]:failed to disarm. Reset sent");
                iot.sendReset("OFF_FAIL");
        }
}
uint8_t get_systemState()
{
        if (digitalRead(SYSTEM_STATE_ALARM_PIN) == SYSTEM_STATE_ON) /* Check if alarm pin is ON */
        {
                return ALARMING;
        }
        else
        {
                if (digitalRead(SYSTEM_STATE_ARM_PIN) == SYSTEM_STATE_ON) /* Check if Armed pin is ON */
                {
                        if (digitalRead(SET_SYSTEM_ARMED_HOME_PIN) == STATE_ON) /* set by code TO home_Armed ?*/
                        {
                                return ARMED_HOME_CODE;
                        }
                        else if (digitalRead(SET_SYSTEM_ARMED_AWAY_PIN) == STATE_ON) /* set by code TO away_Armed ?*/
                        {
                                return ARMED_AWAY_CODE;
                        }
                        else
                        {
                                return ARMED_KEYPAD;
                        }
                }
                else
                {
                        return DISARMED;
                }
        }
}
void set_armState(uint8_t req_state)
{
        uint8_t curState = get_systemState();
        if (req_state != curState)
        {
                if (req_state == DISARMED)
                {
                        if (curState == ARMED_KEYPAD)
                        {
                                digitalWrite(SET_SYSTEM_ARMED_HOME_PIN, STATE_ON);
                                delay(systemPause * 1000); // Time for system to react to fake state change
                        }
                        allOff();
                }
                else if (req_state == ARMED_HOME_CODE || req_state == ARMED_AWAY_CODE)
                {
                        uint8_t armstates[2] = {0, 0};
                        if (req_state == ARMED_HOME_CODE)
                        {
                                armstates[0] = SET_SYSTEM_ARMED_HOME_PIN;
                                armstates[1] = SET_SYSTEM_ARMED_AWAY_PIN;
                        }
                        else
                        {
                                armstates[1] = SET_SYSTEM_ARMED_HOME_PIN;
                                armstates[0] = SET_SYSTEM_ARMED_AWAY_PIN;
                        }

                        if (curState == DISARMED)
                        {
                                digitalWrite(armstates[0], STATE_ON); // Switch to desired arm state
                                delay(systemPause * 1000);
                        }
                        else
                        {
                                char a[50];
                                digitalWrite(armstates[1], !STATE_ON); // verify not in that state
                                delay(systemPause * 1000);
                                digitalWrite(armstates[0], STATE_ON); // switch to desired arm state
                                sprintf(a, "System change: [Disarmed] using [Code]");
                                iot.pub_msg(a);
                        }
                }
        }
        else
        {
                char a[50];
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
                        delay(systemPause * 1000);
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
                char a[100];
                uint8_t curState = get_systemState();
                sprintf(a, "System state: [%s]", sys_states[curState]);
                iot.pub_msg(a);
                iot.pub_state(sys_states[curState]);

                if (curState == DISARMED) /* Verify all inputs are OFF */
                {
                        allOff();
                }
        }
}
void check_systemState_alarming()
{
        if (_detect_state_change(SYSTEM_STATE_ALARM_PIN, indication_ALARMED_lastState))
        {
                uint8_t curstate = get_systemState();
                iot.pub_state(sys_states[curstate]);

                if (curstate == ALARMING)
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
        startGPIOs();
        startIOTservices();
}
void loop()
{
        iot.looper();
        check_systemState_armed();
        check_systemState_alarming();
}
