#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

// ==================== start of TUNEABLE PARAMETERS ====================
const uint16_t kRecvPin = D5; // D5 on a NodeMCU board
const uint32_t kBaudRate = 115200;
//const uint16_t kCaptureBufferSize = 128;
const uint16_t kMinUnknownSize = 12;
// ==================== end of TUNEABLE PARAMETERS ====================

unsigned long key_value = 0;

IRrecv irrecv(kRecvPin);//, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  Serial.begin(kBaudRate);//, SERIAL_8N1, SERIAL_TX_ONLY);
//  while (!Serial)  // Wait for the serial connection to be establised.
//    delay(50);
//  Serial.println();
  Serial.print("IRrecv is running and waiting for IR input on Pin ");
  Serial.println(kRecvPin);

#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                  // DECODE_HASH
  irrecv.enableIRIn();  // Start the receiver
}

void recvIRinputs() {
  if (irrecv.decode(&results)) {

    if (results.value == 0XFFFFFFFF)
      results.value = key_value;

    switch (results.value) {
      case 0xFFA25D:
        Serial.println("CH-");
        break;
      case 0xFF629D:
        Serial.println("CH");
        break;
      case 0xFFE21D:
        Serial.println("CH+");
        break;
      case 0xFF22DD:
        Serial.println("|<<");
        break;
      case 0xFF02FD:
        Serial.println(">>|");
        break ;
      case 0xFFC23D:
        Serial.println(">|");
        break ;
      case 0xFFE01F:
        Serial.println("-");
        break ;
      case 0xFFA857:
        Serial.println("+");
        break ;
      case 0xFF906F:
        Serial.println("EQ");
        break ;
      case 0xFF6897:
        Serial.println("0");
        break ;
      case 0xFF9867:
        Serial.println("100+");
        break ;
      case 0xFFB04F:
        Serial.println("200+");
        break ;
      case 0xFF30CF:
        Serial.println("1");
        break ;
      case 0xFF18E7:
        Serial.println("2");
        break ;
      case 0xFF7A85:
        Serial.println("3");
        break ;
      case 0xFF10EF:
        Serial.println("4");
        break ;
      case 0xFF38C7:
        Serial.println("5");
        break ;
      case 0xFF5AA5:
        Serial.println("6");
        break ;
      case 0xFF42BD:
        Serial.println("7");
        break ;
      case 0xFF4AB5:
        Serial.println("8");
        break ;
      case 0xFF52AD:
        Serial.println("9");
        break ;
    }
    key_value = results.value;
    irrecv.resume();
  }

}

void loop() {
  recvIRinputs();
  delay(100);
}
