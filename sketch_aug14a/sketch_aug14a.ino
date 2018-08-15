byte br[]={'g','u','y','d','v','i','r'};
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
//  c = 
  int a=strlen(br);
  Serial.println(a);
  char c[77];
  for( int i=0; i<3; i++){
//    str(c, char(br[i]));
    c[i]=char(br[i]);
    Serial.println(i);
//    Serial.println(strlen(c));
  }
  Serial.println(c);
    
}

void loop() {
  // put your main code here, to run repeatedly:

}
