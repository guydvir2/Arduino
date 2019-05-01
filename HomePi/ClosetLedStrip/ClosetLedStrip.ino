#define USE_SLEEP true

#if USE_SLEEP
#include <avr/sleep.h>
#endif

#define sensorPin_1 2
#define sensorPin_2 3
#define RelayON HIGH
#define doorOpen HIGH

#define pwrdown_timeOut 10*(1000*60) // mins to powerdown , **0 means noTimeout**

const int relayPin1 = 4;
const int relayPin2 = 5;


class ledDoor {

private:
int _sensorPin;
int _interruptPin;
int _relPin;

public:
bool volatile _doorOpen;
long onCounter=0;
ledDoor(int sensorPin, int relPin){
        _sensorPin=sensorPin;
        _relPin=relPin;

        pinMode(_relPin, OUTPUT);
        pinMode(_sensorPin, INPUT); // sensor io
}
void detection_door(){
    #if USE_SLEEP
        sleep_disable();
    #endif

        detachInterrupt(digitalPinToInterrupt(_sensorPin));
        if (digitalRead(_sensorPin) == doorOpen) {
                _doorOpen = true;
        }
        else {// trun off
                _doorOpen = false;
        }
}

void go2sleep() {
  #if USE_SLEEP
        if (pwrdown_timeOut !=0 && millis()-onCounter >= pwrdown_timeOut && onCounter !=0) {
                sleep_enable();
                set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                sleep_cpu();
        }
        // else{
        //   Serial.println(millis()-onCounter);
        // }
  #endif

}
void switchLeds(){
        if (_doorOpen == true) {
                digitalWrite(_relPin, RelayON);
                onCounter = millis();
        }
        else{
                digitalWrite(_relPin, !RelayON);
                onCounter = 0;
        }
}

};

ledDoor door1(sensorPin_1, relayPin1);
ledDoor door2(sensorPin_2, relayPin2);


void handler_door1(){
        door1.detection_door();
}
void handler_door2(){
        door2.detection_door();
}
void setup() {
        Serial.begin(9600);
        Serial.println("UP");
        attachInterrupt(digitalPinToInterrupt(sensorPin_1), handler_door1, CHANGE);
        attachInterrupt(digitalPinToInterrupt(sensorPin_2), handler_door2, CHANGE);
}

void loop() {
        door1.detection_door();
        door2.detection_door();

        door1.switchLeds();
        door2.switchLeds();

  #if USE_SLEEP
        door1.go2sleep();
        door2.go2sleep();
  #endif
}
