#include <Math.h>
char str[] = "This is my string";  // create a string
  char out_str[40];                  // output from string functions placed here
  int num;                           // general purpose integer
  int max_size=5;
  char new_str[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
void setup() {
  
  
  Serial.begin(9600);
  
//  // (1) print the string
//  Serial.println(str);
//  
//  // (2) get the length of the string (excludes null terminator)
//  num = strlen(str);
//  Serial.print("String length is: ");
//  Serial.println(num);

Serial.print("String length: ");
Serial.println(strlen(new_str));


//Serial.println(num);
//if (num>1){
//  for (int i=0; i<num;i++){
//      char tmp[25];
////    sprintf(tmp,"part #%d",i);
////    strcat(tmp,(String)new_str[i]);
//    for (int m=0; m<max_size;m++){
//      if (i*max_size +m <strlen(new_str)){
////        strcat(tmp,new_str[m]);//new_str[m]);
////          Serial.print(new_str[m]);
////            tmp[m]=(char)new_str[m+i*max_size];
//            tmp[m]=(char)(m+i*max_size);
//
//      }
//      else {
//        tmp[m]='\0';
//        break;
//      }
////      tmp[m]='\0';
//      
//    }
//    Serial.println(tmp);
////    Serial.println("");
//  }    
//  }
// 
msgSplitter(new_str, 5, "Part #"); 
}
int msgSplitter( const char* msg_in, int chop_size, char *prefix char *output[100]){
  num=ceil((float)strlen(msg_in)/chop_size);
  for (int k=0; k<num;k++){
    char tmp[50];
    int pre_len;
    sprintf(tmp,"%s %d: ",prefix ,k);
    pre_len = strlen(tmp);
    for (int i=0; i<chop_size;i++){
      tmp[i+pre_len]=(char)msg_in[i+k*chop_size];
      tmp[i+1+pre_len]='\0';
      }
    Serial.println(tmp);
    }
    return 0;
    }

void loop() {
}
