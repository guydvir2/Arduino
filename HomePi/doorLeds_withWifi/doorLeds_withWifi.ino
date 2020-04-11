#include <myIOT.h>
#include <Arduino.h>

/*
on ESP-01
TX - GPIO1 --> OUTPUT ONLY
RX - GPIO3 --> INPUT  ONLY
*/

// ********** Sketch Services  ***********
#define VER "ESP-01_1.1"
#define Pin_Sensor_0 0
#define Pin_Sensor_1 13 // fake io - not using sensor
#define Pin_Switch_0 2
#define Pin_Switch_1 1
#define Pin_extbut_1 3 // using button to switch on/ off

#define SwitchTimeOUT 30

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL false      // Serial Monitor
#define USE_WDT true          // watchDog resets
#define USE_OTA true          // OTA updates
#define USE_RESETKEEPER false // detect quick reboot and real reboots
#define USE_FAILNTP true      // saves amoount of fail clock updates
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "closetLEDs"
#define MQTT_PREFIX "myHome"
#define MQTT_GROUP "intLights"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************

class SensorSwitch
{
private:
        byte _switchPin, _extPin, _timeout_mins, _sensorPin;

        // PWM settings
        byte _maxLuminVal = 240;
        byte _LumStep = 60;
        byte _PWMstep = 20;
        byte _PWMdelay = 30;
        byte _currentLumVal = _maxLuminVal;

        bool _sensorsState, _last_sensorsState;
        long unsigned _onCounter = 0;
        long unsigned _lastInput = 0;

public:
        float swState = 0.0;
        bool useButton = false;
        bool usePWM = false;
        bool RelayON_def = true;
        bool ButtonPressed_def = LOW;
        bool SensorDetection_def = LOW;

public:
        void turnOff()
        {
                if (usePWM)
                {
                        for (int i = _currentLumVal; i >= 0; i = i - _PWMstep)
                        {
                                analogWrite(_switchPin, i);
                                delay(_PWMdelay);
                        }
                }
                else
                {
                        digitalWrite(_switchPin, !RelayON_def);
                }
                _onCounter = 0;
                swState = 0.0;
        }
        void turnOn()
        {
                if (usePWM)
                {
                        if (_currentLumVal == 0)
                        {
                                _currentLumVal = 0.6 * _maxLuminVal;
                        }
                        for (int i = 0; i <= _currentLumVal; i = i + _PWMstep)
                        {
                                analogWrite(_switchPin, i);
                                delay(_PWMdelay);
                        }
                }
                else
                {
                        digitalWrite(_switchPin, RelayON_def);
                }
                _onCounter = millis();
                swState = 1.0;
        }
        SensorSwitch(byte sensorPin, byte switchPin, byte timeout_mins = 10, byte extPin = 0)
        {
                _sensorPin = sensorPin;
                _switchPin = switchPin;
                _extPin = extPin;
                _timeout_mins = timeout_mins;
        }
        void start()
        {
                pinMode(_sensorPin, INPUT_PULLUP);
                pinMode(_switchPin, OUTPUT);

                if (useButton)
                {
                        pinMode(_extPin, INPUT_PULLUP);
                }

                _sensorsState = digitalRead(_sensorPin);
                _last_sensorsState = digitalRead(_sensorPin);

                turnOff();
        }

        void checkLuminButton()
        {
                if (useButton)
                {
                        if (digitalRead(_extPin) == ButtonPressed_def)
                        {
                                delay(50);
                                if (digitalRead(_extPin) == ButtonPressed_def)
                                {
                                        if (usePWM)
                                        {
                                                if (_currentLumVal - _LumStep >= 0)
                                                {
                                                        _currentLumVal = _currentLumVal - _LumStep;
                                                }
                                                else
                                                {
                                                        _currentLumVal = _maxLuminVal;
                                                }
                                                analogWrite(_switchPin, _currentLumVal);
                                                swState = (float)(_currentLumVal / _maxLuminVal);
                                                delay(200);
                                        }
                                        else
                                        {
                                                if ((int)swState == 0)
                                                {
                                                        turnOff();
                                                }
                                                else
                                                {
                                                        turnOn();
                                                }
                                        }
                                }
                        }
                }
        }
        void looper()
        {
                checkSensor();
                checkLuminButton();
                offBy_timeout();
        }

private:
        void offBy_timeout()
        {
                if (_timeout_mins * 1000ul * 60ul > 0 && _onCounter != 0)
                { // user setup TO ?
                        if (millis() - _onCounter >= _timeout_mins * 1000ul * 60ul)
                        { //TO ended
                                turnOff();
                        }
                }
        }
        void checkSensor()
        {
                _sensorsState = digitalRead(_sensorPin);

                if (_sensorsState != _last_sensorsState)
                { // enter on change only
                        if (millis() - _lastInput > 100)
                        { // ms of debounce
                                if (_sensorsState == !SensorDetection_def)
                                {
                                        turnOn();
                                }
                                else
                                {
                                        turnOff();
                                }
                                _lastInput = millis();
                                _last_sensorsState = _sensorsState;
                        }
                }
        }
};
#define NUM_SW 2
SensorSwitch s0(Pin_Sensor_0, Pin_Switch_0, SwitchTimeOUT);
SensorSwitch s1(Pin_Sensor_1, Pin_Switch_1, SwitchTimeOUT, Pin_extbut_1);
SensorSwitch *s[NUM_SW] = {&s0, &s1};

void startIOTservices()
{
        iot.useSerial = USE_SERIAL;
        iot.useWDT = USE_WDT;
        iot.useOTA = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}

void addiotnalMQTT(char *incoming_msg)
{
        char msg[150];
        char msg2[20];

        if (strcmp(incoming_msg, "status") == 0)
        {
                for (int i = 0; i < NUM_SW; i++)
                {
                        if (s[i]->swState < 1.0 && s[i]->swState > 0.0)
                        {
                                sprintf(msg, "Status: LedStrip [%.0f%%] [On]", s[i]->swState);
                        }
                        else
                        {
                                sprintf(msg, "Status: LedStrip [#%d] [%s]", i, s[i]->swState ? "On" : "Off");
                        }
                }

                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "ver") == 0)
        {
                sprintf(msg, "ver #1: [%s], lib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d]", VER, iot.ver, USE_WDT, USE_OTA, USE_SERIAL, USE_RESETKEEPER, USE_FAILNTP);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "help") == 0)
        {
                sprintf(msg, "Help: Commands #1 - [status, boot, reset, ip, ota, ver, help]");
                iot.pub_msg(msg);
                sprintf(msg, "Help: Commands #2 - [all_off; i,on; i,off]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "all_off") == 0)
        {
                for (int i = 0; i < NUM_SW; i++)
                {
                        if (s[i]->swState != 0.0)
                        {
                                s[i]->turnOff();
                                sprintf(msg, "MQTT: LedStrip [#%d] Turned [Off]", i);
                                iot.pub_msg(msg);
                        }
                }
        }

        else
        {
                int num_parameters = iot.inline_read(incoming_msg);
                int x = atoi(iot.inline_param[0]);

                if (strcmp(iot.inline_param[1], "on") == 0)
                {
                        s[x]->turnOn();
                }
                else if (strcmp(iot.inline_param[1], "off") == 0)
                {
                        s[x]->turnOff();
                }
        }
}
void detectChange()
{
        static float lastval[NUM_SW] = {0.0, 0.0};
        char msg[20];

        for (int i = 0; i < NUM_SW; i++)
        {
                if (s[i]->swState != lastval[i])
                {
                        sprintf(msg, "Switch [#%d] changed to [%.1f]", i, s[i]->swState);
                        iot.pub_msg(msg);
                }
        }
}

void setup()
{
        s0.useButton = false;
        s0.usePWM = false;
        s0.RelayON_def = true;
        s0.ButtonPressed_def = LOW;
        s0.SensorDetection_def = LOW;
        s0.start();

        s1.useButton = true;
        s1.usePWM = false;
        s1.RelayON_def = true;
        s1.ButtonPressed_def = LOW;
        s1.SensorDetection_def = LOW;
        s1.start();

        startIOTservices();
}

void loop()
{
        iot.looper();
        s0.looper();
        s1.looper();
        detectChange();
        delay(100);
}
