char msg[50];
const char* clientID = "Sonoff";
const char *client_temp = "HomePi/Dvir/Windows/";
const char* outTopic = "HomePi/Dvir/Messages";
const char* outTopic2 = "HomePi/Dvir/Windows/ESP32";
const char* inTopic2 = "HomePi/Dvir/Windows/All";


void setup() {
  char inTopic[strlen(client_temp) +strlen(clientID)];
  sprintf(inTopic,"%s%s",client_temp,clientID);
  Serial.begin(9600);
  Serial.println(inTopic);

}

void loop() {
  // put your main code here, to run repeatedly:

}
