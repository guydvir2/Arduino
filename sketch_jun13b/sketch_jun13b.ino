#include <Chrono.h>

Chrono cron(Chrono::SECONDS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Running: ");
  Serial.println(cron.isRunning());
  Serial.print("Elapsed: ");
  Serial.println(cron.elapsed());
  delay(1000);
  cron.stop();
  cron.restart();
//    cron.stop();


}

void loop() {
  // put your main code here, to run repeatedly:
    Serial.print("Running: ");
  Serial.println(cron.isRunning());
  Serial.print("Elapsed: ");
  Serial.println(cron.elapsed());

  delay(1000);

}
