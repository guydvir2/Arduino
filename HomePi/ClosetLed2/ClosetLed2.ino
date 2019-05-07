#include <avr/sleep.h>

#define NUM_SENSORS     1        // <----- NEED TO CHANGE BY USER
#define PWRDOWN_TIMEOUT 1000 * 5 // <----- NEED TO CHANGE BY USER
#define RelayON HIGH


const int sensorPin_1         = 2;
const int sensorPin_2         = 3;
const int relayPin_1          = 4;
const int relayPin_2          = 5;
long unsigned last_input      = 0;
bool volatile doorOpen_1      = 0;
bool volatile doorOpen_2      = 0;

const int sensorsPin[2]       = {sensorPin_1, sensorPin_2};
const int relaysPin[2]        = {relayPin_1, relayPin_2};
bool volatile sensorsState[2] = {false, false};
bool last_sensorsState[2]     = {false, false};
long unsigned onCounters[2]   = {0, 0};
long unsigned lastInputs[2]   = {0, 0};
bool inTimeOUT [2]            = {false, false};


// ~~~~~~ Sleep~~~~~~~~~
void go2sleep(int *sensAmount) {
        Serial.print("go2sleep\n");

        sleep_enable();
        for (int i=0; i<sensAmount; i++) {
                reAttach(i);
        }
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        delay(100);
        sleep_cpu();

        // ~~~~~~~~~~~~~~~~~~~
        Serial.print("wake\n");
        // Serial.println(i);
        // delay(1000);
}
// ~~~~~~~~~~~~~~~~~~~~~

// ~~~ Switching Power ~~~~~~~
void turnOff_relay(int i){
        if (digitalRead(relaysPin[i]) == RelayON) {
                digitalWrite(relaysPin[i], !RelayON);
                last_sensorsState[i]=!RelayON;
                sensorsState[i] = !RelayON;  // need for TO
                onCounters[i] = 0;

                // Serial.print("Off, Sensor #");
                // Serial.println(i);
        }
}
void turnOn_relay(int i){
        if (digitalRead(relaysPin[i])==!RelayON) {
                digitalWrite(relaysPin[i], RelayON);
                last_sensorsState[i] = RelayON;
                sensorsState[i] = RelayON;  // need for TO
                onCounters[i] = millis();

                // Serial.print("On, Sensor #");
                // Serial.println(i);
        }
}
void checkSensor(int i) {
        if (sensorsState[i]!=last_sensorsState[i]) { // enter on change only
                // if (millis()-lastInputs[i]>30) { // ms of debounce
                        if (sensorsState[i] == true) {
                                turnOn_relay(i);
                        }
                        else {
                                turnOff_relay(i);
                        }
                        lastInputs[i] = millis();
                // }
        }
}
void offBy_timeout(int i){
        if (PWRDOWN_TIMEOUT == 0 ) { // user not using TO
                inTimeOUT[i] = false;
        }
        if (PWRDOWN_TIMEOUT > 0) { // user setup TO ?
                if (millis()-onCounters[i]>=PWRDOWN_TIMEOUT) { //TO ended
                        turnOff_relay(i);
                        inTimeOUT[i] = false;
                }
                else { //Still in TO
                        inTimeOUT[i] = true;
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
void sensorActivated(int i) {
        sleep_disable();
        // Serial.print("Door, Sensor #");
        // Serial.println(i);
        // sensorsState[i] = digitalRead(sensorsPin[i]);
        // detachInterrupt(digitalPinToInterrupt(sensorsPin[i]));
}
void sensor0_ISR(){
        // sensorActivated(0);

        sleep_disable();
        detachInterrupt(digitalPinToInterrupt(sensorsPin[0]));
        sensorsState[0] = digitalRead(sensorsPin[0]);
}
void sensor1_ISR(){
        // sensorActivated(1);

        sleep_disable();
        detachInterrupt(digitalPinToInterrupt(sensorsPin[1]));
        sensorsState[1] = digitalRead(sensorsPin[1]);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~ Constructors ~~~~~~~
void startSensors(int i){
        pinMode(relaysPin[i], OUTPUT);
        pinMode(sensorsPin[i], INPUT_PULLUP);
        digitalWrite(relaysPin[i], RelayON);
        last_sensorsState[i] = digitalRead(sensorsPin[i]);
}
void looperSensors(int i){
        checkSensor(i);
        offBy_timeout(i);
        reAttach(i);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~

void setup() {
        // Serial.begin(115200);
        // startSensors(1);
        startSensors(0);
}

void loop() {
        // delay(1000);

        // looperSensors(1);
        looperSensors(0);
        // int sum = 0;
        // for (int i=0; i<NUM_SENSORS; i++) {
        //         sum += inTimeOUT[i];
        // }
        // if (sum == 0){ //inTimeOUT[0]==false && inTimeOUT[1]==false) {
        //   // Serial.println(sum);
        //         go2sleep(NUM_SENSORS);
        // }
        // delay(100);
        // if (inTimeOUT[0]==false && inTimeOUT[1]==false) {
        //   // Serial.println(sum);
        //         go2sleep(NUM_SENSORS);
        // }
        // if (inTimeOUT[0]==false) {
        //         // Serial.println(sum);
        //         go2sleep(NUM_SENSORS);
        // }

}
