class s{
  public:
  s (int a, int b){
    _a=a;
    _b=b;
    
  }

  void adds(){
    Serial.println(_a+_b);
  }

  void decs(){
    Serial.println(_a-_b);
  }

  private:
  int _a, _b;
  
};

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);

s obj1(10,9);
s obj2(100,4);
obj1.adds();
obj2.decs();



}

void loop() {
  // put your main code here, to run repeatedly:

}
