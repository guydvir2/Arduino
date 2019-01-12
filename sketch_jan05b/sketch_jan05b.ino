void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
addiotnalMQTT("1,on");
}

void loop() {
  // put your main code here, to run repeatedly:

}

void addiotnalMQTT(char *incoming_msg) {
  int swNum;
  char cmd[8];
  char tempcmd[20];

  // switch commands via MQTT
  if (isDigit(incoming_msg[0])) {
    if (incoming_msg[1]=='.') {
      Serial.println("YES");
      swNum = incoming_msg[0] - 48;
      for (int i = 2; i < strlen(incoming_msg); i++) {
        cmd[i - 2] = incoming_msg[i];
        cmd[i - 1] = '\0';
      }

    }
  }
}
