template <class T, int N>
class test {

  public:
    test(T a=33){
      
    };
    T arr[N];

    void func() {
      for (int a = 0; a < N; a++) {
        arr[a] = a + N;
      }
    }
    void printosh();


};
template <class T, int N>
void test<T,N>::printosh(){
  for(int i=0;i<N;i++){
    Serial.println(arr[i]);
  }
}

test<int, 1000> test_a;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  test_a.func();
  test_a.printosh();


}

void loop() {
  // put your main code here, to run repeatedly:

}
