#include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>
// #include <TimeLib.h>
// #include <NtpClientLib.h>

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("GUY");
}

void loop(){

}


// // Read GPIO's buttons and relays
//         Rel_0_state = digitalRead(Rel_0_Pin);
//         Rel_1_state = digitalRead(Rel_1_Pin);
// // ######################################
//
// //  MQTT service
//         if (!client.connected()) {
//                 connectMQTT();
//         }
//         client.loop();
// // #########################
//
//
// //  verfiy not in Hazard State
//         if (Rel_0_state == LOW && Rel_1_state == LOW ) {
//                 switchIt("Button","off");
//                 Serial.println("Hazard state - both switches were ON");
//         }
// //  ##
//
// //  physical switch change detected
// //  switch UP
//         if (digitalRead(Sw_0_Pin) != lastSW_0_state) {
//                 delay(50); //debounce
//                 if (digitalRead(Sw_0_Pin) != lastSW_0_state) {
                        // if (digitalRead(Sw_0_Pin) == LOW && Rel_0_state!=LOW) {
//                                 switchIt("Button","up");
//                         }
//                         else if (digitalRead(Sw_0_Pin) == HIGH && Rel_0_state!=HIGH) {
//                                 switchIt("Button","off");
//                         }
//                         else {
//                                 Serial.println("Wrong command");
//                         }
//                 }
//         }
//
// //  switch down
//         if (digitalRead(Sw_1_Pin) != lastSW_1_state) {
//                 delay(50);
//                 if (digitalRead(Sw_1_Pin) != lastSW_1_state) {
//                         if (digitalRead(Sw_1_Pin) == LOW && Rel_1_state!=LOW) {
//                                 switchIt("Button","down");
//                         }
//                         else if (digitalRead(Sw_1_Pin) == HIGH && Rel_1_state!=HIGH) {
//                                 switchIt("Button","off");
//                         }
//                         else {
//                                 Serial.println("Wrong command");
//                         }
//                 }
//         }
//
//         lastSW_0_state = digitalRead(Sw_0_Pin);
//         lastSW_1_state = digitalRead(Sw_1_Pin);
//         delay(50);
// }
