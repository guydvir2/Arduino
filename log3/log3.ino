#include <myLOG.h>
#include <myIOT.h>

flashLOG flog; //("log.txt",5,120);
FVars fvar1;

void setup()
{
    Serial.begin(9600);
    flog.start();
    // flog.read();
    char g[20];// = 0;
    // fvar1.getValue(g);
    // Serial.print("value befire: ");
    // Serial.println(g);
    // fvar1.setValue("GUY");
    // fvar1.format();
    fvar1.getValue(g);
    Serial.print("value after: ");
    Serial.println(g);

    Serial.print("File size: ");
    Serial.println(flog.sizelog());
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (millis() > 10000)
    {
        Serial.println("RESET");
        delay(1000);
        ESP.restart();
    }
    else
    {
        char a[20];
        sprintf(a, "%d", millis());
        flog.write(a);
        flog.postlog(50);
    }
    delay(500);
}
