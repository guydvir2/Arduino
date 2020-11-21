#include <myJSON.h>
String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\
                        \"useFailNTP\" : true,\"useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1}";

myJSON param_on_flash("guy.json", false, 400);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  param_on_flash.start();
  param_on_flash.setValue("key0",(int)7);


}

void loop() {
  // put your main code here, to run repeatedly:

}
