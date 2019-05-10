#define SECONDS               1000
#define MINUTES               SECONDS*60   // [ms]
#define RelayON               HIGH
#define SENSOR_DETECT_DOOR    true

#define NUM_SENSORS           2          // <----- NEED TO CHANGE BY USER
#define USE_SLEEP             true       // <----- NEED TO CHANGE BY USER
#define PWRDOWN_TIMEOUT       5*SECONDS  // <----- NEED TO CHANGE BY USER

#if USE_SLEEP
#include <avr/sleep.h>
#endif

const int sensorPin_1         = 2;
const int sensorPin_2         = 3;
const int relayPin_1          = 13;
const int relayPin_2          = 5;
bool volatile doorOpen_1      = 0;
bool volatile doorOpen_2      = 0;

const int sensorsPin[2]       = {sensorPin_1, sensorPin_2};
const int relaysPin[2]        = {relayPin_1, relayPin_2};
bool volatile sensorsState[2] = {false, false};
bool last_sensorsState[2]     = {false, false};
long unsigned onCounters[2]   = {0, 0};
long unsigned lastInputs[2]   = {0, 0};

void go2sleep() {

#if USE_SLEEP
        if (onCounters[0] == 0 && onCounters[1] == 0) {
                sleep_enable();
                set_sleep_mode(SLEEP_MODE_PWR_DOWN); //SLEEP_MODE_PWR_SAVE);
                Serial.println("Going2Sleep");
                delay(100);
                sleep_cpu();

                Serial.println("WAKE");
                delay(100);
                reAttach(0);
                reAttach(1);
        }
#endif
}

// ~~~ Switching Power ~~~~~~~
void turnOff_relay(int i){
        if (digitalRead(relaysPin[i]) == RelayON) {
                digitalWrite(relaysPin[i], !RelayON);
                onCounters[i] = 0;
                Serial.print("Off, Sensor #");
                Serial.println(i);
        }
}
void turnOn_relay(int i){
        if (digitalRead(relaysPin[i])==!RelayON) {
                digitalWrite(relaysPin[i], RelayON);
                onCounters[i] = millis();
                Serial.print("On , Sensor #");
                Serial.println(i);
        }
}
void checkSensor(int i) {
        if (sensorsState[i]!=last_sensorsState[i]) { // enter on change only
                if (millis()-lastInputs[i] > 100) { // ms of debounce
                  #if USE_SLEEP
                        sleep_disable();
                        #endif
                        if (sensorsState[i] == !SENSOR_DETECT_DOOR ) {
                                turnOn_relay(i);
                        }
                        else {
                                turnOff_relay(i);
                        }
                        lastInputs[i] = millis();
                        last_sensorsState[i]=sensorsState[i];
                }
        }
}
void offBy_timeout(int i){
        if (PWRDOWN_TIMEOUT > 0 && onCounters[i]!=0) { // user setup TO ?
                if (millis() - onCounters[i] >= PWRDOWN_TIMEOUT) { //TO ended
                        turnOff_relay(i);

                        // ~ faking sensor value to shut down using timeout ~~
                        sensorsState[i] = SENSOR_DETECT_DOOR; // mean led off
                        last_sensorsState[i] = SENSOR_DETECT_DOOR;
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                        Serial.print("TimeOut #");
                        Serial.println(i);
                }
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ Interrupts ~~~~~~~~
void reAttach(int i){
        if (i==0) {
                attachInterrupt(digitalPinToInterrupt(sensorsPin[i]), sensor0_ISR, CHANGE);
        }
        if (i==1) {
                attachInterrupt(digitalPinToInterrupt(sensorsPin[i]), sensor1_ISR, CHANGE);
        }
}
void sensor0_ISR(){
        detachInterrupt(digitalPinToInterrupt(sensorsPin[0]));
        sensorsState[0] = digitalRead(sensorsPin[0]);
}
void sensor1_ISR(){
        detachInterrupt(digitalPinToInterrupt(sensorsPin[1]));
        sensorsState[1] = digitalRead(sensorsPin[1]);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~ Constructors ~~~~~~~
void startSensors(int m){
        for (int i=0; i<m; i++) {
                pinMode(relaysPin[i], OUTPUT);
                pinMode(sensorsPin[i], INPUT_PULLUP);

                sensorsState[i] = digitalRead(sensorsPin[i]);
                last_sensorsState[i] = digitalRead(sensorsPin[i]);
                reAttach(i);
                turnOff_relay(m);

                Serial.print("Sensor #");
                Serial.print(i);
                Serial.println(" initiated");
        }
}
void looperSensors(int m){
        for (int i=0; i<m; i++) {
                checkSensor(i);
                offBy_timeout(i);
                reAttach(i);
        }
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() {
        Serial.begin(9600);
        delay(50);
        startSensors(NUM_SENSORS);
}

void loop() {
        looperSensors(NUM_SENSORS);
        go2sleep();
        delay(100);

}
