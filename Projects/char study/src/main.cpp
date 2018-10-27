#include <Arduino.h>

void setup() {
  char like[] = "I like coffee and cake";  // create a string

  Serial.begin(9600);

  // (1) print the string
  Serial.println(like);

  // (2) delete part of the string
  like[13] = 0;
  Serial.println(like);

  // (3) substitute a word into the string
  like[13] = ' ';  // replace the null terminator with a space
  like[18] = 't';  // insert the new word
  like[19] = 'e';
  like[20] = 'a';
  like[21] = 0;    // terminate the string
  Serial.println(like);
}

// void setup() {
//     // put your setup code here, to run once:
// }
//
void loop() {
    // put your main code here, to run repeatedly:
}
