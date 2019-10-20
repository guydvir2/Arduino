int pwmPin = 6;
int x = 0;

void setup() {
        pinMode(pwmPin, OUTPUT);
        Serial.begin(9600);
}

void loop() {
        Serial.println("0");
        analogWrite(pwmPin, 0);
        delay(1000);
        Serial.println("127");
        analogWrite(pwmPin, 127);
        delay(1000);
        Serial.println("255");
        analogWrite(pwmPin, 255);
        delay(1000);
}
