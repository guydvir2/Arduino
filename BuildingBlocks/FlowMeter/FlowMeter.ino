#include <myIOT.h>
#include <Arduino.h>
#include <TimeLib.h>



// ********** Sketch Services  ***********
#define VER              "WemosMini_2.2"
#define USE_BOUNCE_DEBUG false
#define USE_INPUTS       false
#define USE_DAILY_TO     true


// ********** myIOT Class ***********
//~~~~~ Services ~~~~~~~~~~~
#define USE_SERIAL       false
#define USE_WDT          true
#define USE_OTA          true
#define USE_RESETKEEPER  true
#define USE_FAILNTP      true
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~ MQTT Topics ~~~~~~
#define DEVICE_TOPIC "flowMeter"
#define MQTT_PREFIX  "myHome"
#define MQTT_GROUP   ""
// ~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
// ***************************


// ~~~~ ResetKeeper Vars ~~~~~~~
bool badReboot        = false;
bool checkbadReboot   = true;
bool boot_overide     = true;

// ~~~~~~~~ state Vars ~~~~~~~~
#define RelayOn          HIGH
#define SwitchOn         LOW

bool swState [NUM_SWITCHES];
bool last_swState [NUM_SWITCHES];
bool inputs_lastState[NUM_SWITCHES];
//####################################################


// device state definitions
#define ledON LOW
#define TIMEOUT_WARNING  1000*10
const int sensorInterrupt = D2;
//##########################


// ~~~~~ OnLine measuring ~~~~~~~
float calibrationFactor  = 4.5; // pulses per second per litre/minute of flow.
volatile byte pulseCount = 0;
float flowRate           = 0;
int flow_milLiters       = 0;
long total_milLitres     = 0;
unsigned long oldTime    = 0;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~ Cumalatibe consumption
byte currentDay;
byte currentMonth;
int currentYear;
float currentDay_flow = 0; //liters
float lastDay_flow    = 0; //liters
float adHoc_flow      = 0; //liters
float monthly_consumption [12] = {0,0,0,0,0,0,0,0,0,0,0,0}; //liters
const char *months [12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char* systemStates[2]   = {"idle", "flowing"};
char* lastDetectState="";

// ~~~~~~ Flags ~~~~~~~~~
bool adHoc_flag  = false;
bool warningFlag = false;
bool flow_state  = false;
// ~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~ Clock Counters ~~~~~~~~~~~
unsigned long startFlow_clock      = 0;
unsigned long stopFlow_clock       = 0;
unsigned long lastTimeFlow_clock   = 0;
unsigned long currentFlow_duration = 0;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#define ADD_MQTT_FUNC addiotnalMQTT
myIOT iot(DEVICE_TOPIC);
void startIOTservices(){
        iot.useSerial      = USE_SERIAL;
        iot.useWDT         = USE_WDT;
        iot.useOTA         = USE_OTA;
        iot.useResetKeeper = USE_RESETKEEPER;
        iot.resetFailNTP   = USE_FAILNTP;
        strcpy(iot.prefixTopic, MQTT_PREFIX);
        strcpy(iot.addGroupTopic, MQTT_GROUP);
        iot.start_services(ADD_MQTT_FUNC);
}

void setup(){
        startGPIOs();
        startIOTservices();

        delay(1000);
        // ~~~ time on Boot ~~~~~
        time_t t = now();
        currentDay = day(t);
        currentMonth = month(t)-1;
        currentYear = year(t);
}


void startGPIOs(){
        pinMode(sensorInterrupt,INPUT);
        digitalWrite(sensorInterrupt, HIGH);
        attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);
}

//  ~~~~ MQTT messages ~~~~~~~
void addiotnalMQTT(char incoming_msg[50]){
        char state[5];
        char state2[5];
        char msg[100];
        char msg2[100];

        if (strcmp(incoming_msg, "status") == 0) {

                sprintf(msg, "Status: Relay:[%s], Switch:[%s]", state, state2);
                iot.pub_msg(msg);
        }

        else if (strcmp(incoming_msg, "flow") == 0 ) {
                float f_lit = (float)total_milLitres/1000;
                sprintf(msg, "Total: %.02f liters",f_lit );
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "start_count") == 0 ) {
                adHoc_flag = true;
                adHoc_flow = 0;
                iot.pub_msg("Start counter");
        }
        else if (strcmp(incoming_msg, "stop_count") == 0 ) {
                adHoc_flag = false;
                iot.pub_msg("Stop counter");
        }
        else if (strcmp(incoming_msg, "show_count") == 0 ) {
                sprintf(msg, "Flow count: %.02f liters",adHoc_flow);
                iot.pub_msg(msg);
        }
        else if (strcmp(incoming_msg, "reset_count") == 0 ) {
                adHoc_flow = 0;
                iot.pub_msg("Reset counter");
        }

        // else if (strcmp(incoming_msg, "pins") == 0 ) {
        //         sprintf(msg, "Switch: Up[%d] Down[%d], Relay: Up[%d] Down[%d]", inputUpPin, inputDownPin, outputUpPin, outputDownPin);
        //         iot.pub_msg(msg);
        // }
        else if (strcmp(incoming_msg, "ver") == 0 ) {
                sprintf(msg, "ver:[%s], lib:[%s], WDT:[%d], OTA:[%d], SERIAL:[%d]", VER,iot.ver, USE_WDT, USE_OTA, USE_SERIAL);
                iot.pub_msg(msg);
        }
}

void updateFlow_state(float threshold = 2.0){
        int reset_no_flow_time = 20*1000;

        if (millis()- lastTimeFlow_clock > reset_no_flow_time) {
                flow_state      = false;
                warningFlag     = false;
                stopFlow_clock  = 0;
                startFlow_clock = 0;
        }

        if (flowRate > threshold) {
                //  ~~~~~~~~~~~~~ FLOW IS DETECTED ~~~~~~~~
                // ~~~~~ first time ~~~~~~
                if(flow_state == false) {
                        if (millis()-stopFlow_clock > reset_no_flow_time ||
                            stopFlow_clock == 0 || startFlow_clock == 0) {         //reset after no-flow time
                                startFlow_clock = millis();
                                Serial.println("First Time Flow");
                        }
                        flow_state = true;
                        iot.pub_state(systemStates[1]);
                }
        }
        // ~~~~~~~~~~~~~~~~~~~ towards stop flowing ~~~~~~~~~~~~~
        else {
                if(flow_state == true) {         // update only once after flow detect
                        iot.pub_state (systemStates[0]);
                        stopFlow_clock = millis();

                        flow_state  = false;
                        warningFlag = false;
                }
        }
        if (millis()-startFlow_clock > TIMEOUT_WARNING &&
            stopFlow_clock != 0) {         // check allowed time for flowing
                flow_alert();
        }

        lastTimeFlow_clock = millis();
}
void print_OL_readings(){
        if (USE_SERIAL == true) {
                unsigned int frac;
                // Print the flow rate for this second in litres / minute
                Serial.print("Flow rate: ");
                Serial.print(int(flowRate)); // Print the integer part of the variable
                Serial.print("."); // Print the decimal point
                // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
                frac = (flowRate - int(flowRate)) * 10;
                Serial.print(frac, DEC); // Print the fractional part of the variable
                Serial.print("L/min");
                // Print the number of litres flowed in this second
                Serial.print("  Current Liquid Flowing: "); // Output separator
                Serial.print(flow_milLiters);
                Serial.print("mL/Sec");

                // Print the cumulative total of litres flowed since starting
                Serial.print("  Output Liquid Quantity: "); // Output separator
                Serial.print(total_milLitres);
                Serial.println("mL");
        }
}

// ~~~~~~~Measure flow ~~~~~~~~
void pulseCounter(){
        pulseCount++;
}
void measureFlow(){
        if((millis() - oldTime) > 1000 && pulseCount >0) { // calc cycle
                // Disable the interrupt while calculating flow rate
                detachInterrupt(digitalPinToInterrupt(sensorInterrupt));
                flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
                oldTime = millis();
                // convert to millilitres.
                flow_milLiters = (flowRate / 60) * 1000;
                // Add the millilitres passed in this second to the cumulative total
                total_milLitres += flow_milLiters;

                updateFlow_state();
                totalFlow_counter();
                // print_OL_readings();
                pulseCount = 0;
                attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);
        }
}
void totalFlow_counter(){
        time_t t = now();

        // Day use
        if (day(t)==currentDay) {
                currentDay_flow += (float)flow_milLiters/1000;
        }
        else {
                lastDay_flow = (float)currentDay_flow;
                currentDay_flow = (float)flow_milLiters/1000;
                currentDay = day(t);
        }

        // monthly use
        if (month(t) == currentMonth) {
                monthly_consumption[month(t)-1] += (float)flow_milLiters/1000;
        }
        else{
                monthly_consumption[month(t)-1] = (float)flow_milLiters/1000;
                currentMonth = month(t);
        }

        // Counter use
        if (adHoc_flag == true) {
                adHoc_flow += (float)flow_milLiters/1000;
        }

        // Report
        if (USE_SERIAL) {
                Serial.print("Current Day:");
                Serial.println(currentDay_flow);

                // for (int i=0; i<=11; i++) {
                //         Serial.print(months[i]);
                //         Serial.print(": ");
                //         Serial.print(monthly_consumption[i]);
                //         Serial.println(" [liters]");
                // }
                //
                // Serial.print("adHoc_Counter: ");
                // Serial.println(adHoc_flow);
        }
}
void flow_alert(){
        warningFlag = true;
        Serial.println("Alert- overFlowing: ");
}
void loop(){
  
        iot.looper();
        measureFlow();
        delay(100);
}
