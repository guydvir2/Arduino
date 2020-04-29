
#define RE_TRIGGER_DELAY 20

const int trigPin = D7;
const int echoPin = D1;

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
// bool check_detect2()
// {
//     static bool in_detection = false;
//     static long det_clock = 0;
//     const int timeout = 5; //sec
//     byte sample = 2;
//     byte thres_dist = 5;
//     bool alert = false;
//     int sensor_read[sample];

//     for (int a = 0; a < sample; a++)
//     {
//         sensor_read[a] = readSensor();
//         Serial.println(sensor_read[a]);
//         if (a > 0 && abs(sensor_read[a] - sensor_read[a - 1]) > thres_dist)
//         {
//             if (alert == false)
//             {
//                 alert = true; //first value - that is far away
//             }
//             else
//             {
//                 alert = false; // this is detection ( not first time )
//                 Serial.println("DET?");
//             }
//         }
//         // if (alert == true && )
//         //     delay(200);
//     }

//     if (millis() - det_clock >= timeout * 1000 && det_clock > 0)
//     {
//         det_clock = 0;
//         in_detection = false;
//         Serial.println("ZERO");
//     }

//     if (abs(sensor_read[1] - sensor_read[0]) >= thres_dist && in_detection != true)
//     {
//         in_detection = true;
//         det_clock = millis();
//         // Serial.println("detection");
//         return 1;
//     }
//     else
//     {
//         // Serial.println("NOTHING");
//         return 0;
//     }
// }
// void check_3()
// {
//     int arr_size = 10;
//     int arr[arr_size];
//     Serial.println("±±±±±±±±±±±±±±±±±±");
//     for (int a = 0; a < arr_size; a++)
//     {
//         arr[a] = readSensor();
//         if (arr[a] > 350)
//         {
//             arr[a] = 350;
//         }
//         Serial.println(arr[a]);
//         delay(200);
//     }
//     Serial.println("+++++++++++++++++");

//     for (int a = 0; a < arr_size - 2; a++)
//     {
//         if (abs(arr[a] - arr[a + 1]) > 5)
//         {
//             if (abs(arr[a] - arr[a + 2]) > 5)
//             {
//                 Serial.print("DET: ");
//                 Serial.println(arr[a]);
//             }
//             else
//             {
//                 Serial.print("reject: ");
//                 Serial.println(arr[a + 1]);
//                 // Serial.print("value ");
//                 // Serial.print(a + 1);
//                 // Serial.print("replaced with value ");
//                 // Serial.println(a);

//                 // arr[a + 1] = arr[a];
//             }
//         }
//     }
// }
bool check_4(const int dist_threshold = 3, int no_trig_delay=2)
{
    static int last_read = readSensor();
    static long detect_clock = 0;
    int curr_read = readSensor();

    if (abs(last_read - curr_read) < dist_threshold)
    {
        last_read = curr_read;
        return 0;
    }
    else
    {
        delay(100);
        int sec_read = readSensor();
        if (abs(last_read - sec_read) < dist_threshold)
        {
            Serial.println("reject values: ");
            Serial.print(last_read);
            Serial.print("; ");
            Serial.print(curr_read);
            Serial.print("; ");
            Serial.println(sec_read);
            return 0;
        }
        else if (abs(curr_read - sec_read) < dist_threshold && millis() - detect_clock > no_trig_delay*1000)
        {
            Serial.println("DETECTION");
            detect_clock = millis();
            return 1;
        }
        else
        {
            Serial.println("UNKNOWN");
            return 0;
        }
    }
}
void setup()
{
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
    Serial.begin(9600);       // Starts the serial communication
    Serial.println("\nStart!");
}
void loop()
{
    // check_detect2();
    // if (check_detect2())
    // {
    //     Serial.println("Detection");
    // }
    // check_3();

    check_4();
    delay(100);
}
