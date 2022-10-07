#define DBG_MODE true

#if DBG_MODE
#define LOCAL_PARAM true

char fakePin[] = "{\"inputPins\": [19,17,16,5,4,2,23,18,15,14,13,12], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";

char fakeP[] = "{\"entityType\": [0],\
                    \"virtCMD\": [\"\",\"\",\"myHome\/Lights\/int\/gFloor\/SW0\",\"myHome\/Lights\/int\/gFloor\/SW1\",\"myHome\/Windows\/gFloor\/W0\",\"\",\"\",\"\",\"\",\"\"],\
                    \"SW_buttonTypes\": [1,2,2,2],\
                    \"WextInputs\": [0,0],\
                    \"SW_RF\": [0,1,2,3],\
                    \"SW_timeout\": [20,15,0,0],\
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