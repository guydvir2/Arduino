
#include <myLOG.h>
flashLOG flog;
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  flog.start();
  flog.write("WWWWWWWWWWWWWWHHHHHHHHHHHHAAAAAAAAAAAAAAAAAAT");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(200);

}
