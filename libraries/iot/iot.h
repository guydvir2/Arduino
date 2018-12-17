/*
  by_guy
*/
#ifndef iot_h
#define iot_h


#include "Arduino.h"

class iot
{
  public:
    iot(char *devTop);
    // void startNetwork();
    // void network_check();
    // int networkStatus();
    // void pub_msg(char *inmsg);

  private:
    void selectNetwork();
    // void startNTP();
    // void get_timeStamp();
    // void feedTheDog();
    // void acceptOTA();
    // int sendReset(char *header);
    // void startOTA();
    // void startMQTT();
    // int subscribeMQTT();
    // void createTopics(const char *devTopic, char *state, char *avail);
    // void callback(char* topic, byte* payload, unsigned int length);
    // void msgSplitter( const char* msg_in, int max_msgSize, char *prefix, char *split_msg);



};

#endif
