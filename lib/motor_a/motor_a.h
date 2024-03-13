#pragma once

#include<angle.h>
#include<MA.h>
#include<line.h>
#define motor_max 3     //移動平均で使う配列の大きさ
extern void OLED_moving();

class motor_attack{
    public:
        motor_attack();
        void moveMotor_L(angle ang,int val,double ac_val,LINE line);
        void moveMotor_0(angle ang,int val,double ac_val,int flag);
        void motor_0();
        void motor_ac(float);
        int line_val = 2;
        int NoneM_flag = 0;
        float Moutput(int,float);
    private:
        const int pah[4] = {2,4,6,8};
        const int ena[4] = {3,5,7,9};
        double mSin[4] = {1,1,-1,-1};  //行列式のsinの値
        double mCos[4] = {1,-1,-1,1};  //行列式のcosの値
        MA Motor[4];
};