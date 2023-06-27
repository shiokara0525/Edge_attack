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
            if(abs(dir) < 85){
                if(tim_cam.read_ms() < 200){
                    P = 1.3 * (150 - x);
                }
                else{
                    P = 0.75 * (150 - x);
                }
                
            }
            else{
                P = -dir * 1.3;
            }
        }
    }
    else{
        if(B != 0){
            B = 0;
            ac_terget = dir;
            tim_cam.reset();
        }
        if(100 < tim_cam.read_ms()){
            ac_terget = 0;
        }
        else if(tim_cam.read_ms() < 200){
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