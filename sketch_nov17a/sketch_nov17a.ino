#include<myIOT2.h>

myIOT2 iot;
void startIOTservices()
{
  iot.useSerial = true;
  iot.useWDT = true;
  iot.useOTA = true;
  iot.useResetKeeper = true;
  iot.resetFailNTP = true;
  iot.useDebug = true;
  iot.debug_level = 0;
  iot.useNetworkReset = true;
  iot.noNetwork_reset = true;
  strcpy(iot.deviceTopic, "deviceTopic");
  strcpy(iot.prefixTopic, "myHome");
  strcpy(iot.addGroupTopic, "groupTopic");

  iot.start_services(addiotnalMQTT);
}
void addiotnalMQTT(char *incoming_msg)
{
  char msg[150];
  char msg2[20];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "Status: OK");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "help2") == 0)
  {
    sprintf(msg, "Help: Commands #3 - [NEW]");
    iot.pub_msg(msg);
  }
  else if (strcmp(incoming_msg, "ver2") == 0)
  {
    sprintf(msg, "Ver: Ver:%s", "VER");
    iot.pub_msg(msg);
  }
}


void setup() {
  // put your setup code here, to run once:
  startIOTservices();
  String x = "dghfgfhgfgh";
  iot.pub_email(&x);
  //  struct tm tt;
  //  getLocalTime(&tt);
  //  Serial.print("THIS IS: ");
  //  Serial.println(&tt);
}

void loop() {
  // put your main code here, to run repeatedly:
  iot.looper();
//  time_t epo;
//
//  Serial.println(time(&epo));
}
