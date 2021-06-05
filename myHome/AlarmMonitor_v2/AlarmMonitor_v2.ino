
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
// ~~~~ HW Pins and States ~~~~
#define INPUT1 D3  //  Indication system is Armed
#define INPUT2 D4  //  Indication system is Alarmed
#define OUTPUT1 D6 //   (Set system)  armed_Home
#define OUTPUT2 D5 //   (Set system)  Armed_Away

#define RelayOn HIGH
#define SwitchOn LOW
#define VER "NodeMCU_3.67"

#include <myIOT2.h>
#include "myIOT_settings.h"
#include <Arduino.h>

byte relays[] = {OUTPUT1, OUTPUT2};
byte inputs[] = {INPUT1, INPUT2};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// GPIO status flags
bool indication_ARMED_lastState;
bool indication_ALARMED_lastState;

const int systemPause = 2000; // milli-seconds, delay to system react
const int deBounceInt = 50;

void startGPIOs()
{
        for (int i = 0; i < 2; i++)
        {
                pinMode(relays[i], OUTPUT);
                pinMode(inputs[i], INPUT_PULLUP);
        }
        indication_ARMED_lastState = digitalRead(inputs[0]);
        indication_ALARMED_lastState = digitalRead(inputs[1]);
}

void allOff()
{
        for (int i = 0; i < 2; i++)
        {
                digitalWrite(relays[i], !RelayOn);
        }
        delay(systemPause);
}
void arm_home()
{
        if (digitalRead(OUTPUT1) == !RelayOn)
        { // verify it is not in desired state already
                if (digitalRead(OUTPUT2) == RelayOn)
                { // in armed away state
                        digitalWrite(OUTPUT2, !RelayOn);
                        iot.pub_msg("System change: [Disarmed] [Away] using [Code]");
                        delay(systemPause);
                }

                digitalWrite(OUTPUT1, RelayOn); // Now switch to armed_home
                delay(systemPause);

                if (digitalRead(INPUT1) == SwitchOn)
                {
                        iot.pub_msg("System change: [Armed] [Home] using [Code]");
                        iot.pub_state("armed_home");
                }
                else
                {
                        allOff();
                        iot.pub_msg("System change: failed to [Armed] [Home]");
                }
        }
        else
        {
                iot.pub_msg("System change: already in [Armed] [Home]");
        }
}
void arm_away()
{
        if (digitalRead(OUTPUT2) == !RelayOn)
        {
                if (digitalRead(OUTPUT1) == RelayOn)
                { // armed home
                        digitalWrite(OUTPUT1, !RelayOn);
                        iot.pub_msg("System change: [Disarmed] [Home] using [Code]");
                        delay(systemPause);
                }

                digitalWrite(OUTPUT2, RelayOn); // now switch to Away
                delay(systemPause);

                if (digitalRead(INPUT1) == SwitchOn)
                {
                        iot.pub_msg("System change: [Armed] [Away] using [Code]");
                        iot.pub_state("armed_away");
                }
                else
                {
                        allOff();
                        iot.pub_msg("System change: failed to [Armed] [Away]");
                }
        }
        else
        {
                iot.pub_msg("System change: already in [Armed] [Away]");
        }
}
void disarmed()
{
        bool armed_code;
        char mqttmsg[50];

        if (indication_ARMED_lastState == SwitchOn)
        { // indicatio n system is armed
                if (digitalRead(OUTPUT2) == RelayOn || digitalRead(OUTPUT1) == RelayOn)
                { // case A: armed using code
                        allOff();
                        armed_code = true;
                        delay(systemPause);
                }
                else
                { // case B: armed using keyPad
                        // initiate any arm state in order to disarm
                        digitalWrite(OUTPUT1, RelayOn);
                        delay(systemPause / 2); // Time for system to react to fake state change
                        allOff();
                        armed_code = false;
                        delay(systemPause / 2);
                }
                if (digitalRead(INPUT1) != SwitchOn)
                { //&& digitalRead(OUTPUT2) != RelayOn && digitalRead(OUTPUT1) != RelayOn) {
                        sprintf(mqttmsg, "System change: [Disarmed] using [Code]. Was [Armed] using [%s]", armed_code ? "Code" : "KeyPad");
                        iot.pub_msg(mqttmsg);
                        iot.pub_state("disarmed");
                }
                else
                {
                        sprintf(mqttmsg, "INPUT1 is [%d], INPUT2 is [%d], OUTPUT1 is [%d], OUTPUT2 is [%d]", digitalRead(INPUT1), digitalRead(INPUT2), digitalRead(OUTPUT1), digitalRead(OUTPUT2));
                        iot.pub_msg(mqttmsg);
                }
        }
}

void check_systemState_armed()
{ // System OUTPUT 1: arm_state
        if (digitalRead(INPUT1) != indication_ARMED_lastState)
        {
                delay(deBounceInt);
                if (digitalRead(INPUT1) != indication_ARMED_lastState)
                {
                        delay(systemPause);

                        indication_ARMED_lastState = digitalRead(INPUT1);
                        if (indication_ARMED_lastState == SwitchOn)
                        { // system is set to armed
                                if (digitalRead(OUTPUT1) == !RelayOn && digitalRead(OUTPUT2) == !RelayOn)
                                {
                                        iot.pub_msg("System state: [Armed] using [KeyPad]");
                                        iot.pub_state("pending");
                                }
                                else if (digitalRead(OUTPUT1) == RelayOn)
                                {
                                        iot.pub_msg("System State: [Armed] [Home] using [Code]");
                                }
                                else if (digitalRead(OUTPUT2) == RelayOn)
                                {
                                        iot.pub_msg("System State: [Armed] [Away] using [Code]");
                                }
                                else
                                {
                                        iot.pub_log("Error Arming system");
                                }
                        }
                        else
                        { // system detected a disarmed indication :
                                if (digitalRead(OUTPUT2) == RelayOn || digitalRead(OUTPUT1) == RelayOn)
                                { // case A: armed using code, but disarmed by keypad
                                        allOff();
                                }
                                if (digitalRead(OUTPUT1) != RelayOn && digitalRead(OUTPUT2) != RelayOn)
                                {
                                        iot.pub_msg("System State: [Disarmed]");
                                        iot.pub_state("disarmed");
                                }
                        }
                }
        }
}
void check_systemState_alarming()
{ // // System OUTPUT 2: alarm_state
        if (digitalRead(INPUT2) != indication_ALARMED_lastState)
        {
                delay(deBounceInt);
                if (digitalRead(INPUT2) != indication_ALARMED_lastState)
                {
                        delay(systemPause);
                        // alarm set off
                        if (digitalRead(INPUT2) == SwitchOn)
                        {
                                iot.pub_msg("[Alarm] is triggered");
                                iot.pub_state("triggered");
                        }
                        // alarm ended
                        else if (digitalRead(INPUT2) == !SwitchOn)
                        {
                                iot.pub_msg("[Alarm] stopped");
                                if (digitalRead(INPUT1) == SwitchOn)
                                {
                                        if (digitalRead(OUTPUT2) == RelayOn)
                                        {
                                                iot.pub_state("armed_away");
                                        }
                                        else if (digitalRead(OUTPUT1) == RelayOn)
                                        {
                                                iot.pub_state("armed_home");
                                        }
                                        else
                                        {
                                                iot.pub_state("pending");
                                        }
                                }
                                else
                                {
                                        iot.pub_state("disarmed");
                                }
                        }
                        indication_ALARMED_lastState = digitalRead(INPUT2);
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
        delay(100);
}
