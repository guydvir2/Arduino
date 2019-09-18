int sesPin     = D8;
int min_Timeout = 15;
int tot_time   = 0;
bool triggered   = HIGH;
bool detect_flag = false;
long det_clock = 0;
long rem_clock = 0;

void setup() {
        Serial.begin(9600);
        pinMode(sesPin, INPUT_PULLUP);
}

void loop() {
        // Serial.println(digitalRead(sesPin));
        if (digitalRead(sesPin)==triggered) {
                delay(50);
                if (digitalRead(sesPin)==triggered ) {
                        if (detect_flag == false) {
                                detect_flag = true;
                                Serial.println("Detection!");
                                det_clock = millis();
                        }
                        else {
                                rem_clock = millis();
                                Serial.println("rem_clock updated");
                        }
                }
        }
        else{
                delay(50);
                if (digitalRead(sesPin) == !triggered) {
                        if (detect_flag == true && (millis() - det_clock)/1000 > min_Timeout) {
                                detect_flag = false;
                                Serial.print("sensor flag is off, after - ");
                                Serial.println((millis()-det_clock)/1000);
                                det_clock = 0;

                        }
                        else if (detect_flag == false && (millis() -rem_clock)/1000 >1.2*min_Timeout && rem_clock != 0) {
                                rem_clock = 0;
                                Serial.println("TO ended");
                        }

                        // if ((millis() - det_clock)/1000 > min_Timeout) {
                        //         Serial.print("min timeout ended");
                        //         Serial.println((millis()-det_clock)/1000);
                        // }
                        //
                        // if((millis() - rem_clock )/1000 >2*min_Timeout && rem_clock!=0) {
                        //         Serial.println("TO ended");
                        //         Serial.println((millis()-det_clock)/1000);
                        //         rem_clock = 0;
                        // }
                }


        }
        delay(500);

}
