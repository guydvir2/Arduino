#include <myIOT.h>
#include <Arduino.h>

/*
TX - GPIO1 --> OUTPUT ONLY
RX - GPIO3 --> INPUT  ONLY
*/

// ********** Sketch Services  ***********
#define VER "ESP-01_1.0"
#define Pin_Sensor_0 0
#define Pin_Sensor_1 0 //3

#define Pin_Switch_0 2
#define Pin_Switch_1 1

#define SwitchTimeOUT 30

// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL true       // Serial Monitor
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
        byte _PWMstep = 20;
        byte _maxLuminVal = 240;
        byte _LumStep = 60;
        byte _PWMdelay = 30;
        byte _currentLumVal = _maxLuminVal;
        
        bool _sensorsState, _last_sensorsState;
        long unsigned _onCounter = 0;
        long unsigned _lastInput = 0;

public:
        int swState = 0;
        bool useButton = false;
        bool usePWM = false;
        bool RelayON_def = true;
        bool ButtonPressed_def = LOW;
        bool SensorDetection_def = LOW;

public:
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
                                                swState = (int)(_currentLumVal / _maxLuminVal);
                                                delay(200);
                                        }
                                        else
                                        {
                                                if (swState)
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
        void ext_command(bool com)
        {
                if (com)
                {
                        turnOn();
                }
                else
                {
                        turnOff();
                }
        }

private:
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
                swState = 0;
        }
        void turnOn()
        {
                if (usePWM)
                {
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
                swState = 1;
        }
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

SensorSwitch s0(Pin_Sensor_0, Pin_Switch_0, SwitchTimeOUT);
SensorSwitch s1(Pin_Sensor_1, Pin_Switch_1, SwitchTimeOUT);

void start_sensSW(SensorSwitch &s)
{
        s.useButton = false;
        s.usePWM = false;
        s.RelayON_def = true;
        s.ButtonPressed_def = LOW;
        s.SensorDetection_def = LOW;

        s.start();
}

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
                if (s1.swState < 1 && s1.swState > 0)
                {
                        sprintf(msg, "Status: LedStrip [%.0f%%] [On]", s1.swState);
                }
                else
                {
                        sprintf(msg, "Status: LedStrip [%s]", s1.swState ? "On" : "Off");
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
        }
        else if (strcmp(incoming_msg, "on") == 0)
        {
                s1.ext_command(1);
                sprintf(msg, "MQTT: Turned [On]");
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "off") == 0)
        {
                s1.ext_command(0);
                sprintf(msg, "MQTT: Turned [Off]");
                iot.pub_msg(msg);
        }
}

void setup()
{
        startIOTservices();
        start_sensSW(s0);
        start_sensSW(s1);
}

void loop()
{
        iot.looper();
        s0.looper();
        s1.looper();
        delay(100);
}
