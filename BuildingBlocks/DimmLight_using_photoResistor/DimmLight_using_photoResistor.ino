int photoSensorPin = A0;
int PWMpin = 6;

// PhotoResistor values are 0-1023
const int min_lum_value   = 150; // defined as total darkness
const int max_lum_value = 700; // defined as max brightness
int photoSensorValue;

// PWM for MOSFET are 0-255
const int maxON_LEDS   = 255;
const int off_LEDS     = 0;
byte setPWM_Value;


void setup() {
        // Serial.begin(9600);
        // Serial.println("Begin");
        pinMode (photoSensorPin, INPUT);
        pinMode(PWMpin, OUTPUT);
}

void loop() {
        // put your main code here, to run repeatedly:
        photoSensorValue = analogRead(photoSensorPin);

        if (photoSensorValue < min_lum_value) { // below this val it is considered as total Darkness -> set to max_Lum
                setPWM_Value = maxON_LEDS;
                Serial.print("MAX Value");
        }
        else if (photoSensorValue >max_lum_value) { // above this val it is considered as max bright-> turn off leds
                setPWM_Value = off_LEDS;
                Serial.print("MIN Value");
        }
        else { // for all values between 
                setPWM_Value = map(photoSensorValue, min_lum_value, max_lum_value, maxON_LEDS, off_LEDS);
        }

        analogWrite(PWMpin, setPWM_Value);


        // Serial.print("OutPut Value: ");
        // Serial.print(float(setPWM_Value/255.0*100));
        // Serial.print("%, Sensor Value: ");
        // Serial.println(photoSensorValue);


        delay(100);
}
