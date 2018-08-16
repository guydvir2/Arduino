byte* br[]={'g','u','y','d','v','i'};
char* result[50];

void byte2char( byte msg_in[], char msg_out[]) {
  int a=strlen(msg_in);
  for( int i=0; i<a; i++){
    msg_out[i]=char(msg_in[i]);
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print(br);
//  for (int i=0;i<strlen(br);i++){
//    Serial.print(br[i]);
//  }
//  byte2char(br, result);
//  Serial.println(result);
////  c = 
//  int a=strlen(br);
//  Serial.println(a);
//  char c[strlen(br)+1];
//  for( int i=0; i<a; i++){
//    c[i]=char(br[i]);
//    Serial.print(i);
//    Serial.print(": ");
//    Serial.println(c[i]);
//  }
//  Serial.print("Char: ");
//  Serial.println(c);
//  Serial.print("total length: ");
//  Serial.println(strlen(c));
    
}

void loop() {
  // put your main code here, to run repeatedly:

}
