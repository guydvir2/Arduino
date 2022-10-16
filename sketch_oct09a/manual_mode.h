#define MAN_MODE true
#define RETAINED_MSG false

#if MAN_MODE
#define LOCAL_PARAM true

#if defined ESP32
char pins[] = "{\"inputPins\": [19,17,16,5,4,2,23,18,15,14,13,12], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
#elif defined ESP8266
char pins[] = "{\"inputPins\": [0,2,13,15,3,1], \"relayPins\": [4,5,14,12],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
#endif

char cont_params[] = "{\"entityType\": [1],\
                    \"SWname\": [\"myHome\/Lights\/int\/gFloor\/Sw_0\",\"myHome\/Lights\/int\/gFloor\/Sw_1\",\"\",\"\"],\
                    \"Winname\": [\"myHome\/Windows\/gFloor\/Win_0\",\"myHome\/Windows\/gFloor\/Win_1\",\"\",\"\"],\
                    \"SW_buttonTypes\": [1,1,2,2],\
                    \"WextInputs\": [1,0],\
                    \"RF_2entity\": [0,1,2,3],\
                    \"SW_timeout\": [0,0,0,0],\
                    \"SWvirtCMD\":[0,0,0,0],\
                    \"WinvirtCMD\":[0,0,0,0],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\
                    \"pub_topics\" : [\"myHome\/Cont_Man\/Avail\", \"myHome\/Cont_Man\/State\"],\
                    \"sub_topics\" : [\"myHome\/Cont_Man\",\"myHome\/All\",\"myHome\/lockdown\"],\
                    \"sub_topics_win\" : [\"myHome\/Windows\/gFloor\/Win0\",\"myHome\/Windows\/gFloor\/Win1\",\"myHome\/Windows\/gFloor\/Win2\"],\
                    \"sub_topics_SW\" : [\"myHome\/Lights\/int\/gFloor\/Light0\",\"myHome\/Lights\/int\/gFloor\/Light1\",\"myHome\/Lights\/int\/gFloor\/Light2\",\"myHome\/Light\/int\/gFloor\/Light3\"],\
                    \"sub_topics_win_g\" : [\"myHome\/Windows\",\"myHome\/Windows\/gFloor\"],\
                    \"sub_topics_SW_g\" : [\"myHome\/Lights\",\"myHome\/Lights\/int\",\"myHome\/Lights\/int\/gFloor\"],\
                    \"ver\" : 0.1}";
#endif

bool getPins_manual(JsonDocument &DOC)
{
#if MAN_MODE
    // copyArray(pins,)
    DeserializationError err = deserializeJson(DOC, pins);
    if (!err)
    {
        return 1;
    }
    else
    {
        return 0;
    }
#endif
}
void init_Serial_DBG_MODE()
{
    Serial.begin(115200);
    delay(100);
}
