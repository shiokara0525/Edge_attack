#include<Cam.h>
#include<ac.h>


Cam::Cam(){
    pixy.init();
}


int Cam::getCamdata(float dir,float ball_ang){
    pixy.ccc.getBlocks();

    if(pixy.ccc.numBlocks){
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

        if(50 < abs(ball_ang)){
            P = -dir;    
        }
        else{
            if(abs(dir) < 70){
                P = (150 - x);
            }
            else{
                P = -dir;
            }
        }
    }
    else{
        if(B != 0){
            B = 0;
            ac_terget = dir;
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