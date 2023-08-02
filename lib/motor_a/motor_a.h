#pragma once

#include<angle.h>
#include<MA.h>
#include<line.h>
#define motor_max 3              //移動平均で使う配列の大きさ

class motor_attack{
    public:
        motor_attack();
        void moveMotor_L(angle ang,int val,double ac_val,LINE line);
        void moveMotor_0(angle ang,int val,double ac_val);
        void motor_0();
        void motor_ac(float);
        int line_val = 2;
        int NoneM_flag = 0;
    private:
        const int pah[4] = {4,2,8,6};
        const int ena[4] = {5,3,9,7};
        double mSin[4] = {1,1,-1,-1};  //行列式のsinの値
        double mCos[4] = {1,-1,-1,1};  //行列式のcosの値
        MA Motor[4];
        float Moutput(int,float);
};