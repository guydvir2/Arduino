#if MAN_MODE == true

#if defined ESP32
/* RobotDyn 4 Relay module + RF */
char pins[] = "{\"inputPins\": [23,19,18,17,16,15,14,13,5,4,2], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [3135496,3135492,3135490,3135489],\"RFpin\": 27}";

/* RobotDyn 4 Relay module - NO RF */
// char pins[] = "{\"inputPins\": [23,19,18,17,16,15,14,13,5,4,2], \"relayPins\": [25,26,33,32],\"RF_keyboardCode\": [255,255,255,255],\"RFpin\": 255}";

/* LilyGO 8 Relay module */
// char pins[] = "{\"inputPins\": [22,23,26,27,4,15,14,2], \"relayPins\": [33,32,13,12,21,19,18,5],\"RF_keyboardCode\": [255,255,255,255],\"RFpin\": 255}";

#elif defined ESP8266
char pins[] = "{\"inputPins\": [0,4,13,15,3,1], \"relayPins\": [5,2,14,12],\"RF_keyboardCode\": [255,255,255,255],\"RFpin\": 255}";
#endif

#if PARAM_PRESET == 1 /* 2 Windows */
char cont_params[] = "{\"entityType\": [0,0],\
                    \"SWname\": [],\
                    \"SW_buttonTypes\": [],\
                    \"SW_timeout\": [],\
                    \"SWvirtCMD\":[0,0],\
                    \"Winname\": [\"myHome/Windows/gFloor/Win_0\",\
                    \"myHome/Windows/gFloor/Win_1\"],\
                    \"WextInputs\": [0,0],\
                    \"WinvirtCMD\":[0,0],\
                    \"RF_2entity\": [],\
                    \"v_file\": 0.5}";

char topics[] = "{\"pub_gen_topics\":[\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\":[\"myHome/Cont_1M/Avail\",\"myHome/Cont_1M/State\"],\
                    \"sub_topics\" : [\"myHome/Cont_1M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\":[\"myHome/Windows/gFloor/Win_0\",\"myHome/Windows/gFloor/Win_1\"],\
                    \"sub_topics_win_g\":[\"myHome/Windows\",\"myHome/Windows/gFloor\"],\
                    \"sub_topics_SW\":[],\
                    \"sub_topics_SW_g\" : [],\
                    \"ver\" : 0.1}";
#elif PARAM_PRESET == 18 /* 4 Windows */
char cont_params[] = "{\"entityType\": [0,0,0,0],\
                    \"SWname\": [],\
                    \"SW_buttonTypes\": [],\
                    \"SW_timeout\": [],\
                    \"SWvirtCMD\":[0,0],\
                    \"Winname\": [\"Win_0\",\"Win_1\",\"Win_2\",\"Win_3\"],\
                    \"WextInputs\": [0,0,0,0],\
                    \"WinvirtCMD\":[0,0,0,0],\
                    \"RF_2entity\": [],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\" : [\"myHome/Cont_18M/Avail\", \"myHome/Cont_18M/State\"],\
                    \"sub_topics\" : [\"myHome/Cont_18M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\" : [\"myHome/Windows/2Floor/Win0\",\"myHome/Windows/2Floor/Win1\",\"myHome/Windows/2Floor/Win2\",\"myHome/Windows/2Floor/Win3\"],\
                    \"sub_topics_SW\" : [],\
                    \"sub_topics_win_g\" : [\"myHome/Windows\",\"myHome/Windows/2Floor\"],\
                    \"sub_topics_SW_g\" : [],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 2 /* 4 Switches */
char cont_params[] = "{\"entityType\": [1,1,1,1],\
                    \"SWname\": [\"SW_0\",\"SW_1\",\"SW_2\",\"SW_3\"],\
                    \"SW_buttonTypes\": [2,1,1,1],\
                    \"SW_timeout\": [10,11,12,13],\
                    \"SWvirtCMD\":[0,0,0,0],\
                    \"Winname\": [],\
                    \"WextInputs\": [],\
                    \"WinvirtCMD\":[],\
                    \"RF_2entity\": [255,255,255,255],\
                    \"v_file\": 0.5}";

char topics[] = "{\"pub_gen_topics\":[\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\":[\"myHome/Cont_2M/Avail\", \"myHome/Cont_2M/State\"],\
                    \"sub_topics\":[\"myHome/Cont_2M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\":[],\
                    \"sub_topics_SW\":[\"myHome/Lights/2Floor/sw_0\",\"myHome/Lights/2Floor/sw_1\",\
                                        \"myHome/Lights/2Floor/sw_2\",\"myHome/Lights/2Floor/sw_3\"],\
                    \"sub_topics_win_g\":[],\
                    \"sub_topics_SW_g\":[\"myHome/Lights\",\"myHome/Lights/2Floor\"],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 28 /* 8 Switches */
char cont_params[] = "{\"entityType\": [1,1,1,1,1,1,1,1],\
                    \"SWname\": [\"SW_0\",\"SW_1\",\"SW_2\",\"SW_3\",\"SW_4\",\"SW_5\",\"SW_6\",\"SW_7\"],\
                    \"SW_buttonTypes\": [2,1,1,1,2,1,1,1],\
                    \"SW_timeout\": [10,11,12,13,14,15,16,17],\
                    \"SWvirtCMD\":[0,0,0,0,0,0,0,0],\
                    \"Winname\": [],\
                    \"WextInputs\": [],\
                    \"WinvirtCMD\":[],\
                    \"RF_2entity\": [255,255,255,255,255,255,255,255],\
                    \"v_file\": 0.5}";

char topics[] = "{\"pub_gen_topics\":[\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\":[\"myHome/Cont_28M/Avail\", \"myHome/Cont_28M/State\"],\
                    \"sub_topics\":[\"myHome/Cont_28M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\":[],\
                    \"sub_topics_SW\":[\"myHome/Lights/2Floor/sw_0\",\"myHome/Lights/2Floor/sw_1\",\
                                        \"myHome/Lights/2Floor/sw_2\",\"myHome/Lights/2Floor/sw_3\",\
                                        \"myHome/Lights/2Floor/sw_4\",\"myHome/Lights/2Floor/sw_5\",\
                                        \"myHome/Lights/2Floor/sw_6\",\"myHome/Lights/2Floor/sw_7\"],\
                    \"sub_topics_win_g\" : [],\
                    \"sub_topics_SW_g\" : [\"myHome/Lights\",\"myHome/Lights/2Floor\"],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 3 /* 2_windows 4_Light Switches*/
char cont_params[] = "{\"entityType\": [0,0,1,1,1,1],\
                    \"SWname\": [\"SW_0\",\"SW_1\",\"SW_2\",\"SW_3\"],\
                    \"Winname\": [\"myHome/Windows/tFloor/Win_0\",\"myHome/Windows/tFloor/Win_1\"],\
                    \"SW_buttonTypes\": [2,2,2,2],\
                    \"WextInputs\": [0,0],\
                    \"RF_2entity\": [255,255,255,255],\
                    \"SW_timeout\": [10,10,10,10],\
                    \"SWvirtCMD\":[0,0,0,0],\
                    \"WinvirtCMD\":[0,0],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\" : [\"myHome/Cont_3M/Avail\", \"myHome/Cont_3M/State\"],\
                    \"sub_topics\" : [\"myHome/Cont_3M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\" : [\"myHome/Windows/tFloor/Win_0\",\"myHome/Windows/tFloor/Win_1\"],\
                    \"sub_topics_SW\" : [\"myHome/Lights/ext/SW_0\",\"myHome/Lights/ext/SW_1\",\"myHome/Lights/ext/SW_2\",\"myHome/Lights/ext/SW_3\"],\
                    \"sub_topics_win_g\" : [\"myHome/Windows\",\"myHome/Windows/tFloor\"],\
                    \"sub_topics_SW_g\" : [\"myHome/Lights\",\"myHome/Lights/ext\"],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 4 /* 4 Windows Virtual */
char cont_params[] = "{\"entityType\": [0,0,0,0],\
                    \"SWname\": [],\
                    \"SW_buttonTypes\": [],\
                    \"SW_timeout\": [],\
                    \"SWvirtCMD\":[],\
                    \"Winname\": [\"myHome/Windows/tFloor/Win_0\",\"myHome/Windows/tFloor/Win_1\",\"myHome/Windows/tFloor/Win_2\",\"myHome/Windows/tFloor/Win_3\"],\
                    \"WextInputs\": [0,0,0,0],\
                    \"WinvirtCMD\":[1,1,1,1],\
                    \"RF_2entity\": [0,1,2,3],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\" : [\"myHome/Cont_4M/Avail\", \"myHome/Cont_4M/State\"],\
                    \"sub_topics\" : [\"myHome/Cont_4M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\" : [],\
                    \"sub_topics_SW\" : [],\
                    \"sub_topics_win_g\" : [],\
                    \"sub_topics_SW_g\" : [],\
                    \"ver\" : 0.1}";

#elif PARAM_PRESET == 5 /* 4 Switches + RF */
char cont_params[] = "{\"entityType\": [1,1,1,1],\
                    \"SWname\": [\"myHome/Lights/2Floor/SW_0\",\"myHome/Lights/2Floor/SW_1\",\"myHome/Lights/2Floor/SW_2\",\"myHome/Lights/2Floor/SW_3\"],\
                    \"SW_buttonTypes\": [2,1,1,1],\
                    \"SW_timeout\": [10,11,12,13],\
                    \"SWvirtCMD\":[0,0,0,0],\
                    \"Winname\": [],\
                    \"WextInputs\": [],\
                    \"WinvirtCMD\":[],\
                    \"RF_2entity\": [0,1,2,3],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\":[\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\":[\"myHome/Cont_5M/Avail\", \"myHome/Cont_5M/State\"],\
                    \"sub_topics\":[\"myHome/Cont_5M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\":[],\
                    \"sub_topics_SW\":[\"myHome/Lights/2Floor/sw_0\",\"myHome/Lights/2Floor/sw_1\",\"myHome/Lights/2Floor/sw_2\",\"myHome/Lights/2Floor/sw_3\"],\
                    \"sub_topics_win_g\":[],\
                    \"sub_topics_SW_g\":[\"myHome/Lights\",\"myHome/Lights/2Floor\"],\
                    \"ver\":0.1}";

#elif PARAM_PRESET == 6 /* 4 Switches */
char cont_params[] = "{\"entityType\": [1,1,1,1],\
                    \"SWname\": [\"SW_0\",\"SW_1\",\"SW_2\",\"SW_3\"],\
                    \"SW_buttonTypes\": [2,2,1,1],\
                    \"SW_timeout\": [0,0,10,10],\
                    \"SWvirtCMD\":[0,0,0,0],\
                    \"Winname\": [],\
                    \"WextInputs\": [],\
                    \"WinvirtCMD\":[],\
                    \"RF_2entity\": [255,255,255,255],\
                    \"v_file\": 0.5}";

char topics[] = "{\
                    \"pub_gen_topics\" : [\"myHome/Messages\",\"myHome/log\",\"myHome/debug\"],\
                    \"pub_topics\" : [\"myHome/Cont_6M/Avail\", \"myHome/Cont_6M/State\"],\
                    \"sub_topics\" : [\"myHome/Cont_6M\",\"myHome/All\",\"myHome/lockdown\"],\
                    \"sub_topics_win\" : [],\
                    \"sub_topics_SW\" : [\"myHome/Lights/2Floor/sw_0\",\"myHome/Lights/2Floor/sw_1\",\"myHome/Lights/2Floor/sw_2\",\"myHome/Lights/2Floor/sw_3\"],\
                    \"sub_topics_win_g\" : [],\
                    \"sub_topics_SW_g\" : [\"myHome/Lights\",\"myHome/Lights/2Floor\"],\
                    \"ver\" : 0.1}";

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
