int num = 10;
int *p;
char *nameg="GUYDVIR";
char NAME[]={'g','u','y'};
char *pchar=NAME;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nStart");
  
//  p=&num;
//  *p=13;
//  Serial.print(num);
//  Serial.println(*p);

Serial.println(NAME);
pchar=NAME;
Serial.println(pchar);

}

void loop() {
  // put your main code here, to run repeatedly:

}
