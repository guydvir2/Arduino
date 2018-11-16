#define deviceTopic "HomePi/Dvir/Windows/FamilyRoom"
char nameOTA[50];
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  findName(deviceTopic, nameOTA);
  Serial.println(nameOTA);
}

void findName(const char *inmsg, char *outString) {
  String topic = inmsg;
  int m = 0;

  for (int i = topic.lastIndexOf("/") + 1; i < strlen(deviceTopic); i++) {
    outString[m] = inmsg[i];
    outString[m + 1] = '\0';
    m++;
  }
}
void loop() {
  // put your main code here, to run repeatedly:

}
