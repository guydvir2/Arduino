//#include <avr/sleep.h>

#define RelayON HIGH
#define lightON HIGH

const int pwrdown_timeOut  = 1000 * 10;
#define sensorPin_1 2
//const int sensorPin_2      = 3;
const int relayPin_1       = 4;
const int relayPin_2       = 5;
long onCounter             = 0;
bool volatile doorOpen     = 0;


//void go2sleep() {
//  sleep_enable();
//  attachInterrupt(digitalPinToInterrupt(sensorPin_1), detection_door , LOW);
//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//  Serial.println("Going2Sleep");
//  delay(1000);
////  sleep_cpu();
//

//}

void turnOff_timeout() {
  if (pwrdown_timeOut != 0 && onCounter != 0) {
    if (millis() - onCounter >= pwrdown_timeOut) {
      Serial.println("Sleep!");
      digitalWrite(relayPin_1, !lightON);
    }
  }
  //  else if (  {
  //    sleep_enable();
  //    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //    sleep_cpu();

  //  }
  //  if (pwrdown_timeOut == 0 || onCounter == 0) {
  //    sleep_enable();
  //    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //    sleep_cpu();
  //  }
  //  else if ( millis() - onCounter >= pwrdown_timeOut) {
  //    sleep_enable();
  //    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //    sleep_cpu();
  //    // Serial.println("Sleep!");
  //  }

}

void switchLeds() {
  if (doorOpen == true) {
    digitalWrite(relayPin_1, RelayON);
    onCounter = millis();
    attachInterrupt(digitalPinToInterrupt(sensorPin_1), detection_door , CHANGE);

  }
  else {
    digitalWrite(relayPin_1, !RelayON);
    onCounter = 0;
    attachInterrupt(digitalPinToInterrupt(sensorPin_1), detection_door , CHANGE);

  }
}

void detection_door() {
  Serial.println("Interrrupt Fired");//Print message to serial monitor
  //  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(sensorPin_1));
  doorOpen = digitalRead(sensorPin_1);

  // if (digitalRead(_sensorPin) == doorOpen) {
  //         _doorOpen = true;
  //         Serial.println("OPEN");
  // }
  // else {// trun off
  //         _doorOpen = false;
  //         Serial.println("CLOSE");
  // }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start");


  pinMode(relayPin_1, OUTPUT);
  pinMode(sensorPin_1, INPUT_PULLUP);
  digitalWrite(relayPin_1, !lightON);

  //  attachInterrupt(digitalPinToInterrupt(sensorPin_1), detection_door , CHANGE);
}

void loop() {
  //  go2sleep();
  switchLeds();
  turnOff_timeout();
}

//void Going_To_Sleep() {
//  sleep_enable();//Enabling sleep mode
//  attachInterrupt(0, wakeUp, LOW);//attaching a interrupt to pin d2
//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
//  digitalWrite(LED_BUILTIN, LOW); //turning LED off
//  delay(1000); //wait a second to allow the led to be turned off before going to sleep
//  sleep_cpu();//activating sleep mode
//  Serial.println("just woke up!");//next line of code executed after the interrupt
//  digitalWrite(LED_BUILTIN, HIGH); //turning LED on
//}

//void wakeUp() {
//  Serial.println("Interrrupt Fired");//Print message to serial monitor
//  sleep_disable();//Disable sleep mode
//  detachInterrupt(0); //Removes the interrupt from pin 2;
//}
