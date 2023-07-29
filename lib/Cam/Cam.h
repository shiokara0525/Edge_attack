#pragma once

#include<Pixy2UART.h>
#include<timer.h>
#include<MA.h>

class Cam{
    private:
        Pixy2UART pixy;
        float ac_target = 0;
        int B = 999;
        int A = 0;
    public:
        Cam();
        int color = 2;
        float X;
        float ang;
        float Size;
        int getCamdata();
        void print();
        float P = 0;
        int on = 0;
};