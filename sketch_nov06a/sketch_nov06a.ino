//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "00001";

void setup()
{
  Serial.begin(9600);
  Serial.println("START");
  radio.begin();

  //set the address
  radio.openWritingPipe(address);

  //Set module as transmitter
  radio.stopListening();
}
void loop()
{
  //Send message to receiver
  //  const char text[] = "Hello World";
  //  radio.write(&text, sizeof(text));

  char text[4] = "000\0";
  for (int i = 0; i < 3; i++) {
    text[i] = '1';
    Serial.println(text);
    radio.write(&text, sizeof(text));
    delay(1000);
  }
  delay(1000);

}
