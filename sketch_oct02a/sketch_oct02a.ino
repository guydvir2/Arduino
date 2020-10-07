#include <myLOG.h>
flashLOG LOG;

const int logEntries = 15;
char logentries[logEntries][20];

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);

  LOG.start(logEntries, 100);
  LOG.delog();
  Serial.println("BEGIN");

  for (int i = 0; i < 10; i++)
  {
    char t[20];
    sprintf(t, "start entry: %d", i);
    LOG.write(t);
  }
}

void loop()
{
  static bool done = false;
  static int runner = 0;
  static long last = 0;

  LOG.looper();

  if (millis() - last > 15000)
  {
    for (int x = 0; x < 10; x++)
    {
      char t[20];
      sprintf(t, "new line %d", runner);
      runner++;
      LOG.write(t);
    }
    last = millis();

    for (int u = 0; u < LOG.getnumlines(); u++)
    {
      char a[50];
      LOG.readline(u, a);
      Serial.printf("logEntry[#%d]: %s\n", u, a);
    }
  }

  // if (millis() - LOG.lastUpdate > 5000 && LOG.lastUpdate > 0)
  // {
  //   Serial.println("save to file");
  //   LOG._write2file();

  //   Serial.print("fileSize: ");
  //   Serial.println(LOG.sizelog());
  //   Serial.printf("total line in file: %d\n", LOG.getnumlines());
  //   for (int i = 0; i < LOG.getnumlines(); i++)
  //   {
  //     LOG.readline(i, logentries[i]);
  //     Serial.printf("log entry #%d: %s\n", i, logentries[i]);
  //   }
  //   done = true;
  // }
  // if (millis() > 10000 && done)
  // {
  //   Serial.println("entring new values");
  //   done = false;
  //   for (int i = 0; i < 5; i++)
  //   {
  //     char t[20];
  //     sprintf(t, "new line %d", runner);
  //     runner++;
  //     LOG.write(t);
  //   }
  // }

  // put your main code here, to run repeatedly:
}
