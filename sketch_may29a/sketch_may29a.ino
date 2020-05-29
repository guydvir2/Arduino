#include <myJSON.h>

myJSON mylog("logfile.json", true);

void log_entry(int num, char *log_entry){
  char num_c[10];
  sprintf(num_c,"#%d",num);
  mylog.setValue(num_c,log_entry);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nBegin");
  // mylog.format();
  Serial.print("File mount - ");
  Serial.println(mylog.FS_ok());
  
 

}

void loop() {
  static int a=0;
  static bool x= true;
  while (a<20){
    char b[20];
    sprintf(b,"%.1f sec",(float)millis()/1000);
    log_entry(a, "12345678901234567890123456789012345678901234567890");
    delay(100);
    a++;
    Serial.println(a);
  }
  if (x){
    mylog.printFile();
    x=false;
  }
  delay(100);
  // put your main code here, to run repeatedly:


}
