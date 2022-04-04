#include <mySerialMSG.h>
// #include <SoftwareSerial.h>

const uint8_t arrsize = 4;
char devName[] = {"Wemos2"};
char types[3][6] = {"info", "Act", "error"};
char kw[arrsize][6] = {"Name", "Type", "info", "time"};

// #define DEBUG_SERIAL Serial
// SoftwareSerial mySerial(D1, D2); // RX, TX
#define COMM_SERIAL Serial
mySerialMSG SerialComm(devName, COMM_SERIAL);

void init_serialMSG()
{
  COMM_SERIAL.begin(9600);

  SerialComm.KW[0] = kw[0];
  SerialComm.KW[1] = kw[1];
  SerialComm.KW[2] = kw[2];
  SerialComm.KW[3] = kw[3];

  SerialComm.usePings = true;
  SerialComm.start(incomeMSG_cb);
}
void incomeMSG_cb(JsonDocument &_doc)
{
  if (strcmp(_doc[kw[1]], "system") == 0)
  {
    serializeJsonPretty(_doc, COMM_SERIAL);
    COMM_SERIAL.println("");
  }
  char a[100];
  sprintf(a, "Rx.Packets:[%d] \tfailed.Rx.Packets:[%d] \tTx.Packets[%d]", SerialComm.RxPackets, SerialComm.failRxPackets, SerialComm.TxPackets);
  COMM_SERIAL.println(a);
}
void gen_random_MSG()
{
  static unsigned long last_send = 0;

  if (millis() - last_send > random(5000, 10000))
  {
    last_send = millis();
    char a[20];
    sprintf(a, "%lu", millis());
    SerialComm.sendMsg(devName, types[0], "millis", a);
  }
}

void setup()
{
  init_serialMSG();
  // DEBUG_SERIAL.begin(115200);
  COMM_SERIAL.println("\n\nStart");
}

void loop()
{
  SerialComm.loop();
  gen_random_MSG();
  delay(50);
}
