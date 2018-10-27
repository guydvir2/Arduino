char str[] = "This is my string";  // create a string
  char out_str[40];                  // output from string functions placed here
  int num;                           // general purpose integer
void setup() {
  
  
  Serial.begin(9600);
  
  // (1) print the string
  Serial.println(str);
  
  // (2) get the length of the string (excludes null terminator)
  num = strlen(str);
  Serial.print("String length is: ");
  Serial.println(num);
  
  // (3) get the length of the array (includes null terminator)
  num = sizeof(str);  // sizeof() is not a C string function
  Serial.print("Size of the array: ");
  Serial.println(num);
  
  // (4) copy a string
  strcpy(out_str, str);
  Serial.println(out_str);
  
  // (5) add a string to the end of a string (append)
  strcat(out_str, " sketch.");
  Serial.println(out_str);
  num = strlen(out_str);
  Serial.print("String length is: ");
  Serial.println(num);
  num = sizeof(out_str);
  Serial.print("Size of the array out_str[]: ");
  Serial.println(num);
}

void loop() {
}
