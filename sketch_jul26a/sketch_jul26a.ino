
#include <myIOT2.h>
#include <timeoutButton.h>

timeout2<3> timeout2_3;

void setup() {
  // put your setup code here, to run once:
  timeout2_3.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  timeout2_3.loop();
}
