/*
  Arduino Wireless Communication Tutorial
      Example 1 - Transmitter Code

  by Dejan Nedelkovski, www.HowToMechatronics.com

  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(D8, D0); // CE, CSN
const byte address[6] = "00001";

void startRadio_sending() {
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MIN; RF24_PA_LOW
  radio.setRetries(2, 15);
  radio.setChannel(35);
  radio.setAutoAck(0);

  radio.openWritingPipe(address);
  radio.stopListening();
}
void transmit_runTime() {
  unsigned long runMillis = millis();
  unsigned long allSeconds = millis() / 1000;
  int runHours = allSeconds / 3600;
  int secsRemaining = allSeconds % 3600;
  int runMinutes = secsRemaining / 60;
  int runSeconds = secsRemaining % 60;

  char buf[21];
  sprintf(buf, "Remote: %02d:%02d:%02d", runHours, runMinutes, runSeconds);
  radio.write(&buf, sizeof(buf));
  Serial.println(buf);
}
void setup() {
  Serial.begin(9600);
  startRadio_sending();

  Serial.print("Chip_OK: ");
  Serial.println(radio.isChipConnected());
}

void loop() {
  transmit_runTime();
  delay(1000);

  //  const char text[] = "Hello World";
  //  radio.write(&text, sizeof(text));
  //  delay(1000);
}
