// Update variable below:
const char *directionn = "up";
const char *clientID = "Sonoff";

//Consts
const char *client_temp = "HomePi/Dvir/Windows/";
const char *outTopic = "HomePi/Dvir/Messages";
//const char* outTopic2 = "HomePi/Dvir/Windows/ESP32";
const char *inTopic2 = "HomePi/Dvir/Windows/All";
char inTopic[50];



void setup() {
  Serial.begin(9600);
  delay(10);
//  Serial.println("GUY");
  createTopic(client_temp,clientID , inTopic);
  int a=strlen(inTopic);
  Serial.println(a);

}

void createTopic(const char *chr1, const char *chr2, char *result_char) {
  strcpy(result_char, chr1);
  strcat(result_char, chr2);
}

void loop() {
  // put your main code here, to run repeatedly:

}
