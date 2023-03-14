#include<Arduino.h>

class angle{
    public:
        angle(double,bool);
        angle(double,bool,double,bool);
        void setAng(double,bool);
        double to_range(double,bool);
        float degrees;
        double radians;
        void operator=(double);
        void operator-=(double);
        void operator+=(double);
};