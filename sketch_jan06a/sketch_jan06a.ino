#define A "GUY"
#define B "DVIR"
char *T[]= {nullptr, nullptr };
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BEGIN");
  T[0] = A;

  uint8_t a = sizeof(T) / sizeof(char*);
  Serial.println(a);
  for (int i = 0; i < a; i++) {
    Serial.println(T[i]);
    if (T[i] == nullptr) {
      Serial.println("YES");
    }
    else {
      Serial.println("NO");
    }
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
