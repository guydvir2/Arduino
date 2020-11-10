//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(D3, D4);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "00001";
int pins[3] = {2, 3, 4};

void process_msg(char *text) {
    Serial.println(text);
  for (int i = 0; i < 3; i++) {
//    Serial.print("as char in ");
//    Serial.print(i);
//    Serial.print(": ");
//    Serial.println(text[i]);
    int a = text[i]-'0';
//    Serial.println(a);
    digitalWrite(pins[i], a);
    //    if(text[i]=='1'){
    //      Serial.println(i);
    //
    //    }
    //    Serial.println(text[i]);
  }
  //  if (strcmp(text, "Hello World") == 0) {
  //    Serial.println("OK");
  //  }
}
void setup()
{
//  while (!Serial);
  Serial.begin(9600);
  Serial.println("Start");

  radio.begin();

  //set the address
  radio.openReadingPipe(0, address);

  //Set module as receiver
  radio.startListening();
  for (int i = 0; i < 3; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void loop()
{
  //Read the data if available in buffer
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    process_msg(text);
    //    if(strcmp(text,"Hello World")==0){
    //      Serial.println("OK");
    //
    //    }

  }
}
