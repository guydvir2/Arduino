#include <myIOT.h>



void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
hardReboot HR1(64);
hardReboot HR2(64);
Serial.println("\nBegin!");
HR1.print_val();
HR1.check_boot(2);
HR2.print_val();
HR2.check_boot(5);
}

void loop() {
  // put your main code here, to run repeatedly:

}
