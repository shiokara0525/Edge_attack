#include<Pixy2UART.h>
#include<timer.h>
#include<MA.h>

class Cam{
    private:
        Pixy2UART pixy;
        timer tim_cam;
        timer cam_tim;
        int color = 2;
        int b_1 = 999;
        int b = 999;
    public:
        Cam();
        float x;
        float ang;
        float size;
        int getCamdata(float,float,int);
        void print();
        float P = 0;
        int flag_1 = 0;
};