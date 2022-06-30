char a[200] = {"dfskjhlkjhldzfkjvdlfkjvzndlfkjvnfgsdfg"};
char b[200] = {"This is SPARTA_ZFDVSZDFVSDFVSDFVMSODVSOREMW$%OMGWOREBVSDFOMBWOMHTWRTBOM"};
char *A[] = {a, b, a, b, a, b, a, b};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\n\nStart");
  Serial.println(A[0]);
  Serial.println(A[1]);
  Serial.println(strlen(A[1]));
  Serial.println(sizeof(A));
  Serial.println(sizeof(A) / sizeof(A[0]));

  for (uint8_t i = 0; i < sizeof(A) / sizeof(A[0]); i++) {
    char aa[200];
    sprintf(aa, "#%d %s", i, A[i]);
    Serial.println(aa);
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
