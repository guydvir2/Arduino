int clockOn_0 [] = {19,0,0};
int clockOn_1 [] = {20,0,0};

int clockOff_0[] = {6,0,0};
int clockOff_1[] = {22,0,0};

int clock_flag_0[] = {0};
int clock_flag_1[] = {0};

int *dailyTO0[] = {clockOn_0,clockOff_0, clock_flag_0};
int *dailyTO1[] = {clockOn_1,clockOff_1, clock_flag_1};
int **test[] = {dailyTO0,dailyTO1};
// int *test2[] = {{clockOn_0,clockOff_0,1},{clockOn_1,clockOff_1,1}};

struct clock_TO {
        int on[3];
        int off[3];
        bool flag;
};

clock_TO TO_0 = {{19,0,0},{6,0,0},1};
clock_TO TO_1 = {{20,0,0},{22,0,0},0};

// TO_0.on1[3] = {19,0,0};
// TO_0.off1[3] = {6,0,0};
// TO_0.flag1 = 1;

void setup() {
        // put your setup code here, to run once:
        Serial.begin(9600);

        Serial.println("\n");
        Serial.println(sizeof(TO_0.on)/sizeof(TO_0.on[0]));

        TO_0.on[3]=10;

//  Serial.print((sizeof(dailyTO0)/sizeof(dailyTO0[0])));
//  Serial.print((sizeof(dailyTO0[0])/sizeof(dailyTO0[0][0])));


        for (int m=0; m<sizeof(test)/sizeof(test[0]); m++) {
                for (int n=0; n<(sizeof(test[0])/sizeof(test[0][0])); n++) {
                        for (int i=0; i<(sizeof(test[0][0])/sizeof(test[0][0][0])); i++) {
                                Serial.println(test[m][n][i]);
                        }
                }
        }


}

void loop() {
        // put your main code here, to run repeatedly:

}
