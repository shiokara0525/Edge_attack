#pragma once

#include<Pixy2UART.h>
#include<timer.h>
#include<MA.h>

class Cam{
    private:
        Pixy2UART pixy;
        timer tim_cam;
        timer cam_tim;
        int b_1 = 999;
        int b = 999;
        float ac_target = 0;
        int flag_b = 999;
    public:
        Cam();
        int color = 2;
        float X;
        float ang;
        float Size;
        int getCamdata(float,float,int);
        void print();
        float P = 0;
        int flag_1 = 0;
        int flag_2 = 0;
        int test = 0;
};