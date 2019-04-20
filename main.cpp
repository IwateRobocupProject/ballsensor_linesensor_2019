#include "mbed.h"

//read instance
DigitalIn IR0(PB_4);//0
DigitalIn IR1(PB_5);//45
DigitalIn IR2(PA_11);//90
DigitalIn IR3(PA_8);//135
DigitalIn IR4(PF_1);//180(-180)
DigitalIn IR5(PF_0);//-135
DigitalIn IR6(PA_1);//-90
DigitalIn IR7(PA_0);//-45

AnalogIn Line_front(PB_0);//0
AnalogIn Line_right(PB_1);//90
AnalogIn Line_back(PA_7);//180(-180)
AnalogIn Line_left(PA_3);//-90

//send instance
Serial UART(PB_6,PB_7,9800);
AnalogOut degree_analog(PA_4);
AnalogOut distance_analog(PA_5);

// value
const float PI = 3.1415926;
const int rate = 1500;//sampling rate
int Ballmin[7];//temp minimum
double deg,Ave,dis,Vx,Vy;
int min(int a, int b);//minimum
void line_read();//read line value for Timer ticker

//timer
Timer time1;
Timer time2;
Timer time3;
Timer time4;
//interrupt
Ticker line_ticker;

//variable for getting line
float f,b1,r,l;
int a,b,c,d;
enum state {
    front,
    back,
    not1,
    right,
    left,
    not2
};
state state1;
state state2;
int prestate1 = 2;
int prestate2 = 5;


////////////////////
/*Line read target*/
const float target = 0.35;
////////////////////

int main()
{
    wait_ms(200);
    time1.start();
    time2.start();
    time3.start();
    time4.start();
    line_ticker.attach(&line_read,0.0004);

    while(1) {
        int Ball[8] = {0,0,0,0,0,0,0,0};

        for(int i = 1; i <= rate; i++) { //get ball sampling
            if(IR0 == 1) {
                Ball[0] += 1;
            }
            if(IR1 == 1) {
                Ball[1] += 1;
            }
            if(IR2 == 1) {
                Ball[2] += 1;
            }
            if(IR3 == 1) {
                Ball[3] += 1;
            }
            if(IR4 == 1) {
                Ball[4] += 1;
            }
            if(IR5 == 1) {
                Ball[5] += 1;
            }
            if(IR6 == 1) {
                Ball[6] += 1;
            }
            if(IR7 == 1) {
                Ball[7] += 1;
            }
        }

        Ballmin[0] = min(Ball[0],Ball[1]);
        Ballmin[1] = min(Ball[2],Ballmin[0]);
        Ballmin[2] = min(Ball[3],Ballmin[1]);
        Ballmin[3] = min(Ball[4],Ballmin[2]);
        Ballmin[4] = min(Ball[5],Ballmin[3]);
        Ballmin[5] = min(Ball[6],Ballmin[4]);
        Ballmin[6] = min(Ball[7],Ballmin[5]);

        Ave = (double)Ballmin[6]/rate;
        
        //ball is found
        if(Ave <= 0.9) {
                    
            Vx = (double)(rate-Ball[0])*1+(double)(rate-Ball[1])*0.71+(double)(rate-Ball[2])*0+(double)(rate-Ball[3])*-0.71+(double)(rate-Ball[4])*-1+(double)(rate-Ball[5])*-0.71+(double)(rate-Ball[6])*0+(double)(rate-Ball[7])*0.71;
            Vy = (double)(rate-Ball[0])*0+(double)(rate-Ball[1])*0.71+(double)(rate-Ball[2])*1+(double)(rate-Ball[3])*0.71+(double)(rate-Ball[4])*0+(double)(rate-Ball[5])*-0.71+(double)(rate-Ball[6])*-1+(double)(rate-Ball[7])*-0.71;
            deg = (atan2(Vy,Vx)*((double)180/PI));
            
            if(Ball[0] == Ballmin[6]) {
                if(deg > 22.5){
                    deg = 22.5;
                }
                else if(deg < -22.5){
                    deg = -22.5;
                }
            } else if(Ball[1] == Ballmin[6]) { //45
                if(deg > 67.5){
                    deg = 67.5;
                }
                else if(deg < 22.5){
                    deg = 22.5;
                }
            } else if(Ball[2] == Ballmin[6]) { //90
                if(deg > 112.5){
                    deg = 112.5;
                }
                else if(deg < 67.5){
                    deg = 67.5;
                }
            } else if(Ball[3] == Ballmin[6]) { //135
                if(deg > 157.5){
                    deg = 157.5;
                }
                else if(deg < 112.5){
                    deg = 112.5;
                }
            } else if(Ball[4] == Ballmin[6]) { //180(-180)
                if(deg > 0) {
                    if(deg < 157.5) {
                        deg = 157.5;
                    }
                } else {
                    if(deg > -157.5) {
                        deg = -157.5;
                    }
                }
            } else if(Ball[5] == Ballmin[6]) { //-135
                if(deg > -112.5){
                    deg = -112.5;
                }
                else if(deg < -157.5){
                    deg = -157.5;
                }
            } else if(Ball[6] == Ballmin[6]) { //-90
                if(deg > -67.5){
                    deg = -67.5;
                }
                else if(deg < -112.5){
                    deg = -112.5;
                }
            } else if(Ball[7] == Ballmin[6]) { //-45
                if(deg > -22.5){
                    deg = -22.5;
                }
                else if(deg < -67.5){
                    deg = -67.5;
                }
            }
        //ball is not found
        } else {
            deg = 0;
            dis = 255;
        }
        
        /*get ball distance*/
        for(int i = 0; i < 8; i++){
            dis += Ball[i];
        }
        dis = dis/(double)(8*rate);
        
        /*analog send*/
        degree_analog = (deg/360 + 0.5);
        distance_analog = dis;
        
        /*UART send*/ 
        uint16_t deg_UART = (uint16_t)deg + 180; 
        UART.putc(210);
        UART.putc((char)(deg_UART >> 8));
        UART.putc((char)(deg_UART));
        UART.putc((char)(dis*255));
        
        /*debug data send*/
        if(UART.readable() > 0){
            char buffer  = UART.getc();
            if(buffer == 'D'){
                UART.putc((char)(255*Line_front.read()));
                UART.putc((char)(255*Line_back.read()));
                UART.putc((char)(255*Line_left.read()));
                UART.putc((char)(255*Line_right.read()));
            }
        }
    }
}


/*Timer ticker Line process*/
void line_read()
{
    while(1) {
        
        /*Line value read*/
        f = Line_front.read();
        b1 = Line_back.read();
        r = Line_right.read();
        l = Line_left.read();

        /*Vertical*/
        if(f > target) {
            a = 1;
            time1.reset();
        }
        if(b1 > target) {
            b = 1;
            time1.reset();
        }

        if(a == 1 && b == 0) {
            state1 = front;
        } else if(a == 0 && b == 1) {
            state1 = back;
        } else if(a == 1 && b == 1) {
            if(prestate1 == front) {
                state1 = front;
            } else if(prestate1 == back) {
                state2 = back;
            }
        }
        if(f > target && b1 > target) {
            time3.reset();
            time2.reset();
        }
        if(time1.read() >= (double)0.20 || time3.read() <= (double)0.20) { //time
            a = 0;
            b = 0;
            state1 = not1;
        }
        prestate1 = state1;



        /*Side*/
        if(r > target) {
            time2.reset();
            c = 1;
        }
        if(l > target) {
            time2.reset();
            d = 1;
        }
        if(c == 1 && d == 0) {
            state2 = right;
        } else if(c == 0 && d == 1) {
            state2 = left;
        } else if(c == 1 && d == 1) {
            if(prestate2 == right) {
                state2 = right;
            } else if(prestate2 == left) {
                state2 = left;
            }
        }
        if(r > target && l > target) {
            time4.reset();
            time1.reset();
        }
        if(time2.read() >= (double)0.15 || time4.read() <= (double)0.20) { //time
            c = 0;
            d = 0;
            state2 = not2;
        }

        prestate2 = state2;

        /*UART_send*/
        UART.putc(200);
        if(state1 == front && state2 == not2) {
            UART.putc('0');
        } else if(state1 == front && state2 == right) {
            UART.putc('1');
        } else if(state1 == not1 && state2 == right) {
            UART.putc('2');
        } else if(state1 == back && state2 == right) {
            UART.putc('3');
        } else if(state1 == back && state2 == not2) {
            UART.putc('4');
        } else if(state1 == back && state2 == left) {
            UART.putc('5');
        } else if(state1 == not1 && state2 == left) {
            UART.putc('6');
        } else if(state1 == front && state2 == left) {
            UART.putc('7');
        } else {
            UART.putc('N');
            break;
        }
    }
}

/*get minimum*/
int min(int a,int b)
{
    if(a < b) {
        return(a);
    } else {
        return(b);
    }
}