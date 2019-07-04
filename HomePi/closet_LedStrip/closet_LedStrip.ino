#define SENSORPIN        2
#define SWITCHPIN        3
#define BUTTONPIN        4
#define PWRDOWN_TIMEOUT  100  // <----- NEED TO CHANGE BY USER

class SensorSwitch {
  #define RelayON               true
  #define ButtonPressed         LOW
  #define SENSOR_DETECT_DOOR    false
  #define SWITCH_DELAY          30

private:
int _switchPin, _extPin, _timeout_mins;
byte step          = 20;
byte maxLuminVal   = 240;
byte currentLumVal = maxLuminVal;
byte LumStep       = 60;

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
        pinMode(_extPin, INPUT_PULLUP);
        pinMode(_switchPin, OUTPUT);

        _sensorsState = digitalRead(SensorPin);
        _last_sensorsState = digitalRead(SensorPin);
        turnOff();
}
void sensor_ISR(){
        detachInterrupt(digitalPinToInterrupt(SensorPin));
        _sensorsState = digitalRead(SensorPin);
}
void checkLuminButton(){
        if (digitalRead(_extPin) == ButtonPressed) {
                delay(50);
                if (digitalRead(_extPin) == ButtonPressed) {
                        if (currentLumVal - LumStep >= 0) {
                                currentLumVal = currentLumVal - LumStep;
                        }
                        else {
                                currentLumVal = maxLuminVal;
                        }
                        analogWrite(_switchPin,currentLumVal);
                }
        }
}
void looper(){
        checkSensor();
        checkLuminButton();
        offBy_timeout();
}

private:
void turnOff(){
        for (int i=currentLumVal; i>=0; i=i-step) {
                analogWrite(_switchPin,i);
                delay(SWITCH_DELAY);
        }
        _onCounter = 0;
        //Serial.println("Off");
}
void turnOn(){
        for (int i=0; i<=currentLumVal; i=i+step) {
                analogWrite(_switchPin,i);
                delay(SWITCH_DELAY);
        }
        _onCounter = millis();
        //Serial.println("On");
}
void offBy_timeout(){
        if (_timeout_mins*1000ul*60ul> 0 && _onCounter!=0) { // user setup TO ?
                if (millis() - _onCounter >= _timeout_mins*1000ul*60ul) { //TO ended
                        turnOff();
                        // ~ faking sensor value to shut down using timeout ~~
                        _sensorsState = SENSOR_DETECT_DOOR; // mean led off
                        _last_sensorsState = SENSOR_DETECT_DOOR;
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                }
                //Serial.println((_timeout_mins*1000ul*60ul-(millis() - _onCounter))/1000);
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

SensorSwitch s1(SENSORPIN,SWITCHPIN,BUTTONPIN,PWRDOWN_TIMEOUT);
void reAttach(){
        attachInterrupt(digitalPinToInterrupt(s1.SensorPin), isr, CHANGE);
}
void isr(){
        s1.sensor_ISR();
        reAttach();
}

void setup() {
        //Serial.begin(9600);
        s1.start();
        reAttach();
}
void loop() {
        s1.looper();
        delay(100);
}
