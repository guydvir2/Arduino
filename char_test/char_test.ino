#include <Math.h>
char str[] = "This is my string";  // create a string
char out_str[40];                    // output from string functions placed here
int num;                             // general purpose integer
int max_len=45;
char new_str[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *deviceTopic = "HomePi/Dvir/Windows/test1";
char stateTopic[50];
char availTopic[50];
const char* deviceName = deviceTopic;

void setup() {
  
        Serial.begin(9600);
        Serial.print("String length: ");
        Serial.println(strlen(new_str));
        Serial.print("Max message Size: ");
        Serial.println(max_len);
        
//        msgSplitter(new_str, max_len, "[time]","Split #");
          createTopics(deviceTopic, stateTopic, availTopic);
          Serial.println(stateTopic);
          Serial.println(availTopic);
          


}
void msgSplitter( const char* msg_in, int max_strlen, char *prefix, char *split_msg){
          char tmp[50];

  if (strlen(prefix) + strlen(msg_in) > max_strlen) {
        int max_chunk = max_strlen - strlen(prefix) - strlen(split_msg);
        num=ceil((float)strlen(msg_in)/max_chunk);
        Serial.print("Number of splits:");
        Serial.println(num);
        int pre_len;
     
                for (int k=0; k<num; k++) {
                  pre_len=0;
                        sprintf(tmp,"%s %s%d: ",prefix,split_msg,k);
                        pre_len = strlen(tmp);
                        for (int i=0; i<max_chunk; i++) {
                                tmp[i+pre_len]=(char)msg_in[i+k*max_chunk];
                                tmp[i+1+pre_len]='\0';
                        }
                        Serial.println(tmp);
                }
        }
        else {
          sprintf(tmp,"%s %s",prefix,msg_in);
          Serial.println(tmp);
          
        }
}

void createTopics(const char *devTopic, char *state, char *avail) {
  sprintf(state,"%s/State",devTopic);
  sprintf(avail,"%s/Avail",devTopic);
}
void loop() {
}
