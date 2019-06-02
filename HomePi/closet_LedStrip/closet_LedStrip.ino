#define SECOND                1000
#define MINUTE                60000
#define NUM_SENSORS           2          // <----- NEED TO CHANGE BY USER
#define PWRDOWN_TIMEOUT       60*MINUTE  // <----- NEED TO CHANGE BY USER


#define RelayON               LOW        // <----- NEED TO CHANGE BY USER
#define SENSOR_DETECT_DOOR    true

const int sensorPin_1         = 2; // Interrupt 0 - for 1st sensor
const int sensorPin_2         = 3; // Interrupt 1 - for 2nd sensor
const int relayPin_1          = 8;//4; // Relay -
const int relayPin_2          = 9;//5;
bool volatile doorOpen_1      = 0;
bool volatile doorOpen_2      = 0;

const int sensorsPin[2]       = {sensorPin_1, sensorPin_2};
const int relaysPin[2]        = {relayPin_1, relayPin_2};
bool volatile sensorsState[2] = {false, false};
bool last_sensorsState[2]     = {false, false};
long unsigned onCounters[2]   = {0, 0};
long unsigned lastInputs[2]   = {0, 0};


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
                turnOff_relay(i);

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
        delay(100);
}
