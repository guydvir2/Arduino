/*
  Example for different sending methods

  https://github.com/sui77/rc-switch/

*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(9600);

  // Transmitter is connected to Arduino Pin #10
  mySwitch.enableTransmit(5);

  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);

  // Optional set pulse length.
  mySwitch.setPulseLength(320);

  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);


}

void transCode(int i) {
  int codes[] = {1000, 2000, 3000, 4000};
  Serial.print("Sending code #");
  Serial.print(i);
  Serial.print(": ");
  Serial.println(codes[i]);
  Serial.println("Start in: ");
  delay(2000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("NOW !!");
  long t = millis();
  while (millis() - t < 10000){
    mySwitch.send(codes[i], 24);
  delay(50);
  //  mySwitch.send(codes[i], 24);
  //  delay(500);
  //  mySwitch.send(codes[i], 24);
  //  delay(500);
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
  int x = 0;
  while (x < 4) {
    transCode(x);
    x++;
    delay(2000);
  }


  //  delay(5000);
}
