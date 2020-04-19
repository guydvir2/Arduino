
#define RE_TRIGGER_DELAY 20

const int trigPin = 9;
const int echoPin = 10;

void detection_cb()
{
        Serial.println("Detct");
}

float arrayofmeasurements(const int arr_size = 10)
{
    int sum = 0;

    for (int a = 0; a < arr_size; a++)
    {
        sum += readSensor();
        delay(10);
    }
    return sum / arr_size;
}

int readSensor()
{
    long duration;
    int distance;
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2;
    return distance;
}

bool check_detect()
{
    const byte dist_sensitivity = 5; // cm
    static long last_detect = 0;
    const byte re_trigger_seconds = RE_TRIGGER_DELAY;

    float mean_a = arrayofmeasurements();
    delay(100);
    float mean_b = arrayofmeasurements();

    if (abs(mean_b - mean_a) > dist_sensitivity && millis() - last_detect >= re_trigger_seconds * 1000)
    {
        detection_cb();
        last_detect = millis();
        return 1;
    }
    else
    {
        return 0;
    }
}
void setup()
{
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
    Serial.begin(9600);       // Starts the serial communication
}
void loop()
{
    check_detect();
}
