#if MAN_MODE == true
#if defined ESP32
char pins[] = "{\"inputPins\": [19,17,16,5,4,2,23,18,15,14,13,12], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
#elif defined ESP8266
char pins[] = "{\"inputPins\": [0,2,13,15,3,1], \"relayPins\": [4,5,14,12],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";
#endif

#if PARAM_PRESET == 1
char cont_params[] = "{\"entityType\": [0,0],\
                    \"SWname\": [],\
                    \"Winname\": [\"myHome\/Windows\/gFloor\/Win_0\",\"myHome\/Windows\/gFloor\/Win_1\"],\
                    \"SW_buttonTypes\": [1,2,2,2],\
                    \"WextInputs\": [0,0],\
                    \"RF_2entity\": [0,1,2,3],\
                    \"SW_timeout\": [5,5,10,10],\
                    \"SWvirtCMD\":[0,0],\
                    \"WinvirtCMD\":[0,0,0],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\
                    \"pub_topics\" : [\"myHome\/Cont_1\/Avail\", \"myHome\/Cont_1\/State\"],\
                    \"sub_topics\" : [\"myHome\/Cont_1\",\"myHome\/All\",\"myHome\/lockdown\"],\
                    \"sub_topics_win\" : [\"myHome\/Windows\/gFloor\/Win_0\",\"myHome\/Windows\/gFloor\/Win1\",\"myHome\/Windows\/gFloor\/Win_2\"],\
                    \"sub_topics_SW\" : [],\
                    \"sub_topics_win_g\" : [\"myHome\/Windows\",\"myHome\/Windows\/gFloor\"],\
                    \"sub_topics_SW_g\" : [],\
                    \"ver\" : 0.1}";
#elif PARAM_PRESET == 2

char cont_params[] = "{\"entityType\": [1,1,1,1],\
                    \"SWname\": [\"myHome\/Lights\/int\/SW_0\",\"myHome\/Lights\/int\/SW_1\",\"myHome\/Lights\/int\/SW_2\",\"myHome\/Lights\/int\/SW_3\"],\
                    \"Winname\": [],\
                    \"SW_buttonTypes\": [1,2,2,2],\
                    \"WextInputs\": [0,0],\
                    \"RF_2entity\": [0,1,2,3],\
                    \"SW_timeout\": [5,5,10,10],\
                    \"SWvirtCMD\":[0,0],\
                    \"WinvirtCMD\":[0,0,0],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\
                    \"pub_topics\" : [\"myHome\/Cont_1\/Avail\", \"myHome\/Cont_1\/State\"],\
                    \"sub_topics\" : [\"myHome\/Cont_1\",\"myHome\/All\",\"myHome\/lockdown\"],\
                    \"sub_topics_win\" : [],\
                    \"sub_topics_SW\" : [\"myHome\/Lights\/int\/SW_0\",\"myHome\/Lights\/int\/SW_1\",\"myHome\/Lights\/int\/SW_2\",\"myHome\/Lights\/int\/SW_3\"],\
                    \"sub_topics_win_g\" : [],\
                    \"sub_topics_SW_g\" : [\"myHome\/Lights\",\"myHome\/Lights\/int\"],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 3
char cont_params[] = "{\"entityType\": [0,0,1,1],\
                    \"SWname\": [\"myHome\/Lights\/int\/SW_0\",\"myHome\/Lights\/int\/SW_1\"],\
                    \"Winname\": [\"myHome\/Windows\/tFloor\/Win_0\",\"myHome\/Windows\/tFloor\/Win_1\"],\
                    \"SW_buttonTypes\": [1,2,2,2],\
                    \"WextInputs\": [0,1],\
                    \"RF_2entity\": [0,1,2,3],\
                    \"SW_timeout\": [5,5,10,10],\
                    \"SWvirtCMD\":[0,0],\
                    \"WinvirtCMD\":[0,0],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome\/Messages\",\"myHome\/log\",\"myHome\/debug\"],\
                    \"pub_topics\" : [\"myHome\/Cont_3\/Avail\", \"myHome\/Cont_3\/State\"],\
                    \"sub_topics\" : [\"myHome\/Cont_3\",\"myHome\/All\",\"myHome\/lockdown\"],\
                    \"sub_topics_win\" : [\"myHome\/Windows\/tFloor\/Win_0\",\"myHome\/Windows\/tFloor\/Win_1\"],\
                    \"sub_topics_SW\" : [\"myHome\/Lights\/int\/SW_0\",\"myHome\/Lights\/int\/SW_1\",\"myHome\/Lights\/int\/SW_2\",\"myHome\/Lights\/int\/SW_3\"],\
                    \"sub_topics_win_g\" : [\"myHome\/Windows\",\"myHome\/Windows\/tFloor\"],\
                    \"sub_topics_SW_g\" : [\"myHome\/Lights\",\"myHome\/Lights\/int\"],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 4

#elif PARAM_PRESET == 5

#endif
#endif
bool getPins_manual(JsonDocument &DOC)
{
#if MAN_MODE
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
