#include <myLOG.h>

flashLOG fLOG;
void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  fLOG.start();
  fLOG.write("HELLLLLO", true);
  fLOG.rawPrintfile();
}

void loop()
{
  // put your main code here, to run repeatedly:
  fLOG.looper();
}
