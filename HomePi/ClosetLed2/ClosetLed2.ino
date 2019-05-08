// #include <avr/sleep.h>
#define SECONDS         1000
#define MINUTES         SECONDS*60  // [ms]
#define NUM_SENSORS     2           // <----- NEED TO CHANGE BY USER
#define PWRDOWN_TIMEOUT 15*MINUTES  // <----- NEED TO CHANGE BY USER
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




// ~~~ Switching Power ~~~~~~~
void turnOff_relay(int i){
        if (digitalRead(relaysPin[i]) == RelayON) {
                digitalWrite(relaysPin[i], !RelayON);
                onCounters[i] = 0;
                // Serial.print("Off, Sensor #");
                // Serial.println(i);
        }
}
void turnOn_relay(int i){
        if (digitalRead(relaysPin[i])==!RelayON) {
                digitalWrite(relaysPin[i], RelayON);
                onCounters[i] = millis();
                // Serial.print("On , Sensor #");
                // Serial.println(i);
        }
}
void checkSensor(int i) {
        if (sensorsState[i]!=last_sensorsState[i]) { // enter on change only
                if (millis()-lastInputs[i]>100) { // ms of debounce
                        if (sensorsState[i] == false) {
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
                digitalWrite(relaysPin[i], RelayON);
                last_sensorsState[i] = digitalRead(sensorsPin[i]);
                reAttach(i);

                // Serial.print("Sensor #");
                // Serial.print(i);
                // Serial.println(" initiated");
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
        // Serial.begin(9600);
        // delay(50);
        startSensors(NUM_SENSORS);
}

void loop() {
        looperSensors(NUM_SENSORS);
        delay(100);
}
