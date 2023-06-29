#include<Cam.h>
#include<ac.h>


Cam::Cam(){
    pixy.init();
}


int Cam::getCamdata(float dir,float ball_ang,int flag){
    pixy.ccc.getBlocks();

    if(pixy.ccc.numBlocks && flag == 0){
        if(B != 1){
            B = 1;
        }
        int num = 0;
        float size_max = 0;

        for (int i = 1; i < pixy.ccc.numBlocks + 1; i++){
            if(pixy.ccc.blocks->m_signature == y_b){
                if(size_max < pixy.ccc.blocks[i].m_width){
                    size_max = pixy.ccc.blocks[i].m_width;
                    num = i;
                }
            }
        }
        x = pixy.ccc.blocks[num].m_x;
        ang = pixy.ccc.blocks[num].m_angle;
        size = size_max;
        float p = 150 - x;

        if(23 < abs(ball_ang)){
            if(B_2 != 0){
                B_2 = 0;
                tim_cam.reset();
            }
            P = -dir;
        }
        else{
            if(B_2 != 1){
                B_2 = 1;
                tim_cam.reset();
            }
            if(abs(dir) < 70){
                if(tim_cam.read_ms() < 200){
                    P = 1.3 * p;
                }
                else{
                    P = 0.75 * p;
                }
                flag_2 = 1;
            }
            else{
                P = -dir * 1.3;
            }
        }
        if(num == 0){
            P = 1.3 * -dir;
        }
    }
    else{
        if(B != 0){
            B = 0;
            ac_terget = 0;
            tim_cam.reset();
        }
        if(tim_cam.read_ms() < 100){
            P = 1.3 * (ac_terget - dir);
        }
        else{
            P = (ac_terget - dir);
        }
    }

    if(pixy.ccc.numBlocks == 0){
        flag_1 = 0;
    }
    else{
        flag_1 = 1;
    }
    return flag_1;
}



void Cam::print(){
    if(flag_1 == 0){
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