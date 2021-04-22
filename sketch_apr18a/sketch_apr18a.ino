#include <myDisplay.h>

#define SENDER false
#define NANO_NUM 1
#if NANO_NUM == 0
#define MCU_NAME "NANO_0"
#define DELAYED_SEND 8.5
#elif NANO_NUM == 1
#define MCU_NAME "NANO_1"
#define DELAYED_SEND 13.95
#endif

char *msg = "OUT_MSG";
char construct_msg[30];

myLCD mLCD(16,2,0x27);

void make_msg()
{
  sprintf(construct_msg, "%s: CLK: %d", MCU_NAME, millis() / 1000);
}
void send_msg(char msg[], byte S)
{
  for (int i = 0; i < S; i++)
  {
    Serial.write(msg[i]);
  }
}
void incomingMSG_handler(String &inmsg)
{
  char buff[30];
  int str_len = inmsg.length() + 1; 
  inmsg.toCharArray(buff,str_len);
  mLCD.clear();
  mLCD.CenterTXT(buff);
}
void get_serial_msg()
{
  if (Serial.available() > 0)
  {
    String inStr = Serial.readString();
    incomingMSG_handler(inStr);
  }
}
void setup()
{
  Serial.begin(115200);
  mLCD.start();
  mLCD.CenterTXT(MCU_NAME);
}

void loop()
{
  if (NANO_NUM >=0 )
  {
    get_serial_msg();
  }
  static unsigned long lastTX = 0;
  if (millis() - lastTX > DELAYED_SEND * 1000)
  {
    make_msg();
    send_msg(construct_msg, strlen(construct_msg));
    lastTX = millis();
  }
  delay(50);
}
