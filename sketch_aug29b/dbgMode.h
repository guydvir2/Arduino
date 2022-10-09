#define DBG_MODE true
#define RETAINED_MSG true

#if DBG_MODE
#define LOCAL_PARAM false
#if defined ESP32
char fakePin[] = "{\"inputPins\": [19,17,16,5,4,2,23,18,15,14,13,12], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
#elif defined ESP8266
char fakePin[] = "{\"inputPins\": [0,2,13,15,3,1], \"relayPins\": [4,5,14,12],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
#endif

char fakeP[] = "{\"entityType\": [0, 1, 1],\
                    \"virtCMD\": [\"myHome\/Windows\/gFloor\/TwinWindow\",\"myHome\/Lights\/int\/gFloor\/Sw_0\",\"myHome\/Lights\/int\/gFloor\/Sw_1\",\"\",\"\",\"\",\"\",\"\"],\
                    \"SW_buttonTypes\": [1,2,2,2],\
                    \"WextInputs\": [0,0],\
                    \"SW_RF\": [0,1,2,3],\
                    \"SW_timeout\": [0,0,0,0],\
                    \"v_file\": 0.4}";

char fakeTopics[] = "{\
                        \"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\
                        \"pub_topics\" : [\"myHome\/Cont_Man\/Avail\", \"myHome\/Cont_Man\/State\"],\
                        \"sub_topics\" : [\"myHome\/Cont_Man\",\"myHome\/All\",\"myHome\/lockdown\"],\
                        \"sub_topics_win\" : [\"myHome\/Windows\/gFloor\/Win0\",\"myHome\/Windows\/gFloor\/Win1\",\"myHome\/Windows\/gFloor\/Win2\"],\
                        \"sub_topics_SW\" : [\"myHome\/Light\/int\/gFloor\/Light0\",\"myHome\/Light\/int\/gFloor\/Light1\",\"myHome\/Light\/int\/gFloor\/Light2\",\"myHome\/Light\/int\/gFloor\/Light3\"],\
                        \"sub_topics_win_g\" : [\"myHome\/Windows\",\"myHome\/Windows\/gFloor\"],\
                        \"sub_topics_SW_g\" : [\"myHome\/Light\",\"myHome\/Light\/int\",\"myHome\/Light\/int\/gFloor\"],\
                        \"ver\" : 0.1}";
#endif