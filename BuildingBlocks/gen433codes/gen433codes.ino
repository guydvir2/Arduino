/*
  Example for different sending methods

  https://github.com/sui77/rc-switch/

*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
#define REMOTE_ID 1

void setup() {

  Serial.begin(9600);

  // Transmitter is connected to Arduino Pin #10
  mySwitch.enableTransmit(4);

  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);

  // Optional set pulse length.
  mySwitch.setPulseLength(320);

  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);


}

void transCode(int i) {
  int codes[] = {3, 7, 11, 13};
  Serial.print("REMOTE_ID: #");
  Serial.println(REMOTE_ID);
  Serial.print("Sending code #");
  Serial.print(i);
  Serial.print(": ");
  Serial.println(codes[i]*REMOTE_ID);
  Serial.println("Start in: ");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("NOW !!");
  long t = millis();
  while (millis() - t < 10000){
    mySwitch.send(codes[i]*REMOTE_ID, 24);
  delay(50);

}
Serial.println("END");

}

void loop() {

  /* See Example: TypeA_WithDIPSwitches */
  //  mySwitch.switchOn("11111", "00010");
  //  delay(1000);
  //  mySwitch.switchOff("11111", "00010");
  //  delay(1000);

  /* Same switch as above, but using decimal code */
  //  mySwitch.send(5393, 24);
  //  delay(1000);
  //  mySwitch.send(5396, 24);
  //  delay(1000);

  //  /* Same switch as above, but using binary code */
  //  mySwitch.send("000000000001010100010001");
  //  delay(1000);
  //  mySwitch.send("000000000001010100010100");
  //  delay(1000);
  //
  //  /* Same switch as above, but tri-state code */
  //  mySwitch.sendTriState("00000FFF0F0F");
  //  delay(1000);
  //  mySwitch.sendTriState("00000FFF0FF0");
  //  delay(1000);
  int x = 3;
  while (x >=0) {
    transCode(x);
    x--;
    delay(1000);
  }


  //  delay(5000);
}
