<<<<<<< HEAD
#define SECOND                1000
#define MINUTE                60000
#define NUM_SENSORS           1          // <----- NEED TO CHANGE BY USER
#define PWRDOWN_TIMEOUT       60*MINUTE  // <----- NEED TO CHANGE BY USER
=======
#define SWITCHPIN        3
#define SENSORPIN        2
#define PWRDOWN_TIMEOUT  60  // <----- NEED TO CHANGE BY USER
>>>>>>> 698d8d2b109d7e81fb31785beae62a412b489ab9

class SensorSwitch {
  #define RelayON               true
  #define SENSOR_DETECT_DOOR    false
  #define SWITCH_DELAY          30

private:
int _switchPin, _extPin, _timeout_mins;
int step = 20;
bool _sensorsState, _last_sensorsState;
long unsigned _onCounter = 0;
long unsigned _lastInput = 0;

public:
int SensorPin;


SensorSwitch(int sensorPin, int switchPin, int extPin, int timeout_mins=10){
        SensorPin=sensorPin;
        _switchPin=switchPin;
        _extPin=extPin;
        _timeout_mins=timeout_mins;
}
void start(){
        pinMode(SensorPin, INPUT_PULLUP);
        // pinMode(_extPin, INPUT_PULLUP);
        pinMode(_switchPin, OUTPUT);

        _sensorsState = digitalRead(SensorPin);
        _last_sensorsState = digitalRead(SensorPin);
        turnOff();
}
void sensor_ISR(){
        detachInterrupt(digitalPinToInterrupt(SensorPin));
        _sensorsState = digitalRead(SensorPin);
}
void looper(){
        checkSensor();
        offBy_timeout();
        // reAttach();
}

private:
void turnOff(){
        if (digitalRead(_switchPin) == RelayON) {
                for (int i=255; i>0; i=i-step) {
                        analogWrite(_switchPin,i);
                        delay(SWITCH_DELAY);
                }
                digitalWrite(_switchPin, !RelayON);
                _onCounter = 0;
                // Serial.print("Off");
        }
}
void turnOn(){
        if (digitalRead(_switchPin) == !RelayON) {
                for (int i=0; i<255; i=i+step) {
                        analogWrite(_switchPin,i);
                        delay(SWITCH_DELAY);
                }
                digitalWrite(_switchPin, RelayON);
                _onCounter = millis();
                // Serial.println("On");
                // Serial.println(_timeout_mins*1000*60ul);
        }
}
void offBy_timeout(){
        if (_timeout_mins*1000*60ul> 0 && _onCounter!=0) { // user setup TO ?
                if (millis() - _onCounter >= _timeout_mins*1000*60ul) { //TO ended
                        turnOff();
                        // ~ faking sensor value to shut down using timeout ~~
                        _sensorsState = SENSOR_DETECT_DOOR; // mean led off
                        _last_sensorsState = SENSOR_DETECT_DOOR;
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                }
        }
}
void checkSensor(){
        if (_sensorsState!=_last_sensorsState) { // enter on change only
                if (millis()-_lastInput > 100) { // ms of debounce
                        if (_sensorsState == !SENSOR_DETECT_DOOR ) {
                                turnOn();
                        }
                        else {
                                turnOff();
                        }
                        _lastInput = millis();
                        _last_sensorsState=_sensorsState;
                }
        }
}

};

SensorSwitch s1(SENSORPIN,SWITCHPIN,8,PWRDOWN_TIMEOUT);
void reAttach(){
        attachInterrupt(digitalPinToInterrupt(s1.SensorPin), isr, CHANGE);
}
void isr(){
        s1.sensor_ISR();
        reAttach();
}

void setup() {
        s1.start();
        reAttach();
}
void loop() {
        s1.looper();
        delay(50);
}
