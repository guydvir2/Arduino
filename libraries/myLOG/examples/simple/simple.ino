#include <myLOG.h>

#define USE_DELAYED_SAVE true
#define DEBUG_MODE true
#define WRITE_PERIOD 10
#define NUM_ENTRIES 1000
#define FILE_NAME "/guyLOG.txt"

flashLOG fLOG(FILE_NAME);

void Create_MSG(char *msg, int delay_millis)
{
  static int counter = 0;
  static unsigned long lastWrte = 0;

  if (millis() - lastWrte > delay_millis)
  {
    lastWrte = millis();
    char a[50];
    sprintf(a, "[#%d][%d] - %s", ++counter, int(millis()/1000), msg);
    fLOG.write(a, false);
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\nStart!");

  fLOG.start(NUM_ENTRIES, USE_DELAYED_SAVE, DEBUG_MODE);
  fLOG.write("This is boot", true);  // using WriteNOW
  fLOG.rawPrintfile();
}

void loop()
{
  // put your main code here, to run repeatedly:
  fLOG.looper(WRITE_PERIOD); // use only when delayed_save is needed
  Create_MSG("NEW ENTRY", 100);
  delay(50);
}
