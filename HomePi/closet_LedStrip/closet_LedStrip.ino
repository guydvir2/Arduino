#define SENSORPIN_1 1//0//0 //3//2
#define SWITCHPIN_1 3//3//2 //2//0
#define BUTTONPIN_1 3

#define SENSORPIN_2 8
#define SWITCHPIN_2 3
#define BUTTONPIN_2 0

#define PWRDOWN_TIMEOUT 30 // mins <----- NEED TO CHANGE BY USER

class SensorSwitch
{
#define RelayON true
#define ButtonPressed LOW
#define SENSOR_DETECT_DOOR LOW
#define SWITCH_DELAY 30

private:
        int _switchPin, _extPin, _timeout_mins;
        byte step = 20;
        byte maxLuminVal = 240;
        byte currentLumVal = maxLuminVal;
        byte LumStep = 60;

        bool _sensorsState, _last_sensorsState;
        long unsigned _onCounter = 0;
        long unsigned _lastInput = 0;

public:
        int SensorPin;
        bool useButton = false;
        bool usePWM = false;

        SensorSwitch(int sensorPin, int switchPin, int extPin, int timeout_mins = 10)
        {
                SensorPin = sensorPin;
                _switchPin = switchPin;
                _extPin = extPin;
                _timeout_mins = timeout_mins;
        }
        void start()
        {
                pinMode(SensorPin, INPUT_PULLUP);

                if (useButton)
                {
                        pinMode(_extPin, INPUT_PULLUP);
                }
                pinMode(_switchPin, OUTPUT);

                _sensorsState = digitalRead(SensorPin);
                _last_sensorsState = digitalRead(SensorPin);
                turnOff();
        }
        // void sensor_ISR()
        // {
        //         detachInterrupt(digitalPinToInterrupt(SensorPin));
        //         _sensorsState = digitalRead(SensorPin);
        // }
        void checkLuminButton()
        {
                if (useButton)
                {
                        if (digitalRead(_extPin) == ButtonPressed)
                        {
                                delay(50);
                                if (digitalRead(_extPin) == ButtonPressed)
                                {
                                        if (currentLumVal - LumStep >= 0)
                                        {
                                                currentLumVal = currentLumVal - LumStep;
                                        }
                                        else
                                        {
                                                currentLumVal = maxLuminVal;
                                        }
                                        analogWrite(_switchPin, currentLumVal);
                                        delay(200);
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
        void turnOff()
        {
                if (usePWM)
                {
                        for (int i = currentLumVal; i >= 0; i = i - step)
                        {
                                analogWrite(_switchPin, i);
                                delay(SWITCH_DELAY);
                        }
                }
                else
                {
                        digitalWrite(_switchPin, !RelayON);
                }
                _onCounter = 0;
        }
        void turnOn()
        {
                if (usePWM)
                {
                        for (int i = 0; i <= currentLumVal; i = i + step)
                        {
                                analogWrite(_switchPin, i);
                                delay(SWITCH_DELAY);
                        }
                }
                else
                {
                        digitalWrite(_switchPin, RelayON);
                }
                _onCounter = millis();
        }
        void offBy_timeout()
        {
                if (_timeout_mins * 1000ul * 60ul > 0 && _onCounter != 0)
                { // user setup TO ?
                        if (millis() - _onCounter >= _timeout_mins * 1000ul * 60ul)
                        { //TO ended
                                turnOff();
                                // ~ faking sensor value to shut down using timeout ~~
                                _sensorsState = SENSOR_DETECT_DOOR; // mean led off
                                _last_sensorsState = SENSOR_DETECT_DOOR;
                                // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        }
                }
        }
        void checkSensor()
        {
                if (_sensorsState != _last_sensorsState)
                { // enter on change only
                        if (millis() - _lastInput > 100)
                        { // ms of debounce
                                if (_sensorsState == SENSOR_DETECT_DOOR)
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

SensorSwitch s1(SENSORPIN_1, SWITCHPIN_1, BUTTONPIN_1, PWRDOWN_TIMEOUT);
// SensorSwitch s2(SENSORPIN_2, SWITCHPIN_2, BUTTONPIN_2, PWRDOWN_TIMEOUT);

// void reAttach_s1()
// {
//         attachInterrupt(digitalPinToInterrupt(s1.SensorPin), isr_s1, CHANGE);
// }
// void isr_s1()
// {
//         s1.sensor_ISR();
//         reAttach_s1();
// }

// void reAttach_s2()
// {
//         attachInterrupt(digitalPinToInterrupt(s2.SensorPin), isr_s2, CHANGE);
// }
// void isr_s2()
// {
//         s2.sensor_ISR();
//         reAttach_s2();
// }

void setup()
{
        Serial.begin(9600);
        Serial.println("BOOT");
        s1.useButton = false;
        s1.usePWM = false;
        s1.start();
        // reAttach_s1();
}
void loop()
{
        s1.looper();
        delay(100);
        Serial.println("LOO");
}
