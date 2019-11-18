int PIR_pin = 8;
int beepPin = 9;
bool lastState = LOW;
bool curState = LOW;


void beep (int x=3){
        digitalWrite(beepPin,HIGH);
        delay(x);
        digitalWrite(beepPin,LOW);
}
void setup() {
        Serial.begin(9600);
        Serial.println("Begin");
        pinMode (PIR_pin, INPUT);
        pinMode (beepPin, OUTPUT);
}

void loop() {
        // put your main code here, to run repeatedly:
        curState = digitalRead(PIR_pin);

        if (curState != lastState) {
                lastState = curState;

                if (curState) {
                        beep();
                        Serial.println("Detection on");
                }
                else{
                        Serial.println("Detection off");
                }
        }

        delay(100);
}
