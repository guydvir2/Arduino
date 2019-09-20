
/*
PIR detector has 3 states:
1) H.W SENSOR DETECT TIME - time that sensors return HIGH upon physical detection - 2-7 seconds.
2) MIN_ON_TIME - time that code will set ON function as HIGH after a detection.
3) TIME_ON_AFTER_DETECTION - if during MIN_ON_TIME, additinal sensing happens, a T.O begins and restarts every detection
*/


#define MIN_ON_TIME 10 // seconds
#define TIME_ON_AFTER_DETECTION 15 //seconds
#define SENS_IS_TRIGGERED HIGH

bool inTriggerMode       = false;
int sensorPin            = D8;
long detection_timestamp = 0;
long timeout_counter     = 0;
int calc_timeout         = 0;
int time_from_detection  = 0;


void on_function(){
        Serial.println("ON");
}
void off_function(){
        Serial.println("OFF");
        detection_timestamp = 0;
        timeout_counter = 0;
}

void setup() {
        Serial.begin(9600);
        Serial.println("\nStart!");
        pinMode(sensorPin, INPUT_PULLUP);

}

void loop() {
        calc_timeout = (millis() - timeout_counter)/1000;

        if (detection_timestamp !=0) {
                time_from_detection = (millis() - detection_timestamp)/1000;
        }
        else {
                time_from_detection = 0;
        }

        // HW senses
        if (digitalRead(sensorPin) == SENS_IS_TRIGGERED) {
                delay(50);
                if (digitalRead(sensorPin) == SENS_IS_TRIGGERED ) {
                        //Sensor in detect Mode 1st time
                        if (inTriggerMode == false && detection_timestamp == 0 && timeout_counter == 0) {
                                inTriggerMode = true;
                                Serial.println("Detection!");
                                detection_timestamp = millis();
                                on_function();
                        }

                        // sensor senses again after sensor is not high - it starts T.O.
                        else if ( inTriggerMode == false ) {
                                timeout_counter = millis();
                        }

                        // very goes into T.O when sensor keeps HW sensing and time is greater than MIN_ON_TIME
                        else if (inTriggerMode == true && time_from_detection > MIN_ON_TIME && detection_timestamp!=0 && timeout_counter == 0) {
                                inTriggerMode = false;
                        }
                }
        }

        // HW sense stops
        else{
                delay(50);
                if (digitalRead(sensorPin) == !SENS_IS_TRIGGERED) {
                        // Notify when HW sense ended
                        if (inTriggerMode == true) {
                                inTriggerMode = false;
                                Serial.print("sensor flag is off, after ");
                                Serial.print(float(time_from_detection));
                                Serial.println("[sec]");
                        }
                        // T.O has ended (greater than minimal time on detection)
                        else if (inTriggerMode == false && timeout_counter != 0 && calc_timeout >TIME_ON_AFTER_DETECTION) {
                                Serial.print("TO ended after: ");
                                Serial.print(float(time_from_detection));
                                Serial.println("[sec]");
                                off_function();
                        }
                        // Minimal time on upon detection
                        else if ( inTriggerMode == false && time_from_detection > MIN_ON_TIME && detection_timestamp!=0 && timeout_counter == 0) {
                                Serial.print("MIN_ON_TIME is over after: ");
                                Serial.print(float(time_from_detection));
                                Serial.println("[sec]");
                                off_function();
                        }
                }
        }
        delay(500);

}
