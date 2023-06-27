#include<Pixy2UART.h>
#include<timer.h>
#include<MA.h>

class Cam{
    private:
        Pixy2UART pixy;
        timer tim_cam;
        float p_old;
        int B = 999;
        float ac_terget;
        int B_2 = 999;
    public:
        Cam();
        float x;
        float ang;
        float size;
        int flag_1;
        int getCamdata(float,float,int);
        void print();
        float P = 0;
};