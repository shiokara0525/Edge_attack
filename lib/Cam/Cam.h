#include<Pixy2UART.h>
#include<MA.h>

class Cam{
    private:
        Pixy2UART pixy;
        float p_old;
        int B;
        float ac_terget;
    public:
        Cam();
        float x;
        float ang;
        float size;
        int flag;
        int getCamdata(float,float);
        void print();
        float P = 0;
};