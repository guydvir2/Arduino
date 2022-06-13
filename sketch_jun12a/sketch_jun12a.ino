#define name1 "ABCD"
#define name2 "EFGH"
#define name3 "HIJK"

char *list[3] = {};

void printList(char *l[], int x) {
  for (int i = 0; i < x; i++) {
    Serial.println(l[i]);
  }

}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("start");
  list[0] = name1;
  list[1] = name2;
  list[2] = name3;

  printList(list, 3);

}

void loop() {
  // put your main code here, to run repeatedly:

}
