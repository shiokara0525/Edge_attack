#include<Cam.h>
#include<ac.h>


Cam::Cam(){
    pixy.init();
}


int Cam::getCamdata(float dir){
    pixy.ccc.getBlocks();

    if (pixy.ccc.numBlocks){
        if(B != 1){
            B = 1;
        }
        int num;
        float size_max = 0;

        for (int i = 0; i < pixy.ccc.numBlocks; i++){
            if(size_max < pixy.ccc.blocks[i].m_width){
                size_max = pixy.ccc.blocks[i].m_width;
                num = i;
            }
        }
        x = pixy.ccc.blocks[num].m_x;
        ang = pixy.ccc.blocks[num].m_angle;
        size = size_max;
        flag = 1;

        P = (150 - x);
    }
    else{
        if(B != 0){
            B = 0;
            if(315 < x){
                ac_terget = dir - 90;
            }
            else if(x < 35){
                ac_terget = dir + 90;
            }
            else{
                ac_terget = 0;
            }
            Serial.print("dir_target : ");
            Serial.print(dir);
        }
        flag = 0;
        P = (ac_terget - dir) * 2;
        Serial.print(dir);
    }
    return flag;
}



void Cam::print(){
    if(flag == 0){
        Serial.println("No block detected");
    }
    else{
        Serial.print("x: ");
        Serial.print(x);
        Serial.print("  ang: ");
        Serial.print(ang);
        Serial.print(" P : ");
        Serial.print(P);
        Serial.print("  size: ");
        Serial.println(size);
    }
}