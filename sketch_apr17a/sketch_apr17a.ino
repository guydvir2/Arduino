#include <myLOG.h>

flashLOG fLOG;

void printlogrecords() {
  byte i = fLOG.getnumlines();
  Serial.print("Log records: ");
  Serial.println(i);
}
void add_entry(int i) {
  char a[10];
  sprintf(a, "#%d", i);
  fLOG.write(a);
}
void printlog() {
  char a[10];
  for (int x = 0; x < fLOG.getnumlines(); x++) {
    fLOG.readline(x, a);
    Serial.println(a);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nStart!");
  fLOG.start(50, 15, false);
  //  fLOG.del_last_record();

  fLOG.rawPrintfile();
}

void loop() {
  // put your main code here, to run repeatedly:
  static int i = 0;
  static unsigned long lastAction = 0;
  while (millis() - lastAction > 10000) {
//    add_entry(i);
//    delay(100);
//    printlog();
//    i++;
    lastAction = millis();
//    printlogrecords();
        fLOG.del_line(2);
        Serial.println("DELTED");
  }
  fLOG.looper(3);
  delay(100);
}
