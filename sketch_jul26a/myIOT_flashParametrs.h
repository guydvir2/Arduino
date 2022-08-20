extern myIOT2 iot;
extern void init_LightButton(JsonDocument &DOC);

#define MAX_TOPIC_SIZE 40          // <----- Verfy max Topic size
#define JSON_FILESIZE 600 * MAX_SW // <----- Verfy Json size

char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"}; // <----- Verfy file names

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[3][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];

void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t ar_size)
{
    for (uint8_t i = 0; i < ar_size; i++)
    {
        strlcpy(ch_array[i], DOC[topic][i] | defaulttopic, MAX_TOPIC_SIZE);
        dest_array[i] = ch_array[i];
    }
}
void update_sketch_parameters_flash(JsonDocument &DOC)
{
    /* Custom paramters for each sketch used IOT2*/
    init_LightButton(DOC);
}
void update_Parameters_flash()
{
    StaticJsonDocument<JSON_FILESIZE> DOC;

    /* Part A: update filenames of paramter files */
    iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0]));

    /* Part B: Read from flash, and update myIOT parameters */
    iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC);
    iot.update_vars_flash_parameters(DOC);
    // serializeJsonPretty(DOC, Serial);
    DOC.clear();

    /* Part C: Read Topics from flash, and update myIOT Topics */
    iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC); /* extract topics from flash */
    updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics", "myHome/Messages", sizeof(topics_gen_pub) / (sizeof(topics_gen_pub[0])));
    updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics", "myHome/log", sizeof(topics_pub) / (sizeof(topics_pub[0])));
    updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics", "myHome/log", sizeof(topics_sub) / (sizeof(topics_sub[0])));
    // serializeJsonPretty(DOC, Serial);
    DOC.clear();

    /* Part D: Read Sketch paramters from flash, and update Sketch */
    iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC);
    update_sketch_parameters_flash(DOC);
    // serializeJsonPretty(DOC, Serial);
    // Serial.flush();
    DOC.clear();
}
