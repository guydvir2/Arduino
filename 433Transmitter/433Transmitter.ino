#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

void setup()
{
    Serial.begin(9600);    // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    const char *msg = "Hello";
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();

//    uint8_t buf[12];
//  uint8_t buflen = sizeof(buf);
//  if (driver.recv(buf, &buflen)) // Non-blocking
//  {
//    int i;
//    // Message with a good checksum received, dump it.
//    Serial.print("Message: ");
//    Serial.println((char*)buf);
//  }
      delay(1000);

  
}
