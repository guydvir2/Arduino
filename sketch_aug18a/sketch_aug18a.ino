#include <myJSON.h>

// myJSON jPar("Jparameters.JSON");
myJSON jPar("/config.JSON");

void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	Serial.println("Begin");

	// char a[20];
	// int b;
	// bool c;

	// if (!jPar.getValue("char_param", a)) {
	// 	Serial.println("fail read char");
	// } else {
	// 	Serial.printf("Char stored is %s\n", a);
	// }
	// if (!jPar.getValue("int_param", b)) {
	// 	Serial.println("fail read int");
	// } else {
	// 	Serial.printf("int stored is %d\n", b);
	// }
	// if(!jPar.getValue("bool_param",c)){
	// 		Serial.println("fail read bool");
	// 	}
	// 	else{
	// 		Serial.printf("bool stored is %d\n",c);
	// 	}

//	jPar.setValue("char_param","value_c");
//	jPar.setValue("int_param",123);
//	jPar.setValue("bool_param", true);

	jPar.printFile();

}

void loop() {
	// put your main code here, to run repeatedly:

}
