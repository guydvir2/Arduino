//#include <Ticker.h>
//
//Ticker ticks;
//volatile int wdtCount=0; 
//
//void ISRwatchdog () {
//  wdtCount ++;
//  if (wdtCount >=5 ){
//    ESP.reset();
//  }
//}
//
//void setup() {
//  Serial.begin(9600);
//  Serial.println("Hi Guy");
//  ticks.attach(1, ISRwatchdog);
//}
//
//void loop() {
//  Serial.printf("Watchdog conter = %d\n",wdtCount);
//  delay(1000);
//}
//#include <EspClass.h>

int k=0;

void setup(){
ESP.wdtDisable();
  while (1) {};

}

void loop() {
  Serial.println("GUY");

}

