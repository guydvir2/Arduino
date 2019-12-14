

/*************************************************************

  This sketch shows how to read values from Virtual Pins

  App project setup:
    Slider widget (0...100) on Virtual Pin V1
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "yyJsC24RBVrsgts59QoZ_LYWj1ZEfx74";
char ssid[] = "Xiaomi_D6C8";
char pass[] = "guyd5161";

BLYNK_WRITE(V1)
{
  long pinValue = param.asLong(); // assigning incoming value from pin V1 to a variable

}
BLYNK_READ(A0)
{
  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
  Blynk.virtualWrite(A0, millis() / 1000);
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
}
