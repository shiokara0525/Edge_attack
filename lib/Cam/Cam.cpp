#include<Cam.h>
#include<ac.h>


Cam::Cam(){
    pixy.init();
}


int Cam::getCamdata(float dir,float ball_ang,int flag){
    pixy.ccc.getBlocks();
    int num = 999;
    int cam_able = 1;
    int size = 0;
    int x = 0;
    flag_1 = 0;
    if(pixy.ccc.numBlocks){
        flag_2 = 1;
        if(b_1 != 1){
            b_1 = 1;
        }
        cam_able = 1;
        for(int i = 0; i < pixy.ccc.numBlocks; i++){
            if(pixy.ccc.blocks[i].m_signature == color){
                if(size < pixy.ccc.blocks[i].m_height){
                    num = i;
                    size = pixy.ccc.blocks[i].m_height;
                    x = pixy.ccc.blocks[i].m_x;
                }
            }
        }
        X = x;
        Size = size;
        if(num == 999 || 23 < abs(ball_ang) || 45 < abs(dir)){
            cam_able = 0;
        }
        if(flag == 1){
            cam_able = 1;
        }
        else if(flag == 2){
            cam_able = 2;
        }
    }
    else{  //カメラ見てないとき
        flag_2 = 0;
        if(b_1 != 0){
            b_1 = 0;
            cam_tim.reset();
        }

        if(cam_tim.read_ms() < 100){
            cam_able = 1;
        }
        else{
            cam_able = 0;
        }
    }

    if(cam_able == 0){
        if(b != 0){
            b = 0;
            tim_cam.reset();
        }

        if(tim_cam.read_ms() < 100 && flag != 2){
            test = 1;
            P = -dir * 2;
        }
        else{
            P = -dir;
            test = 2;
        }
        return 0;
    }
    else if(cam_able == 2){
        if(b != 2){
            b = 2;
            ac_target = dir;
        }

        P = (ac_target - dir);
        return 0;
    }
    else{
        if(b != 1){
            b = 1;
            tim_cam.reset();            
        }

        if(tim_cam.read_ms() < 250){
            P = (150 - x) * 1.00;
            flag_1 = 1;
            test = 3;
        }
        else{
            P = (150 - x) * 0.5;
            flag_1 = 2;
            test = 4;
        }
        return 1;
    }
}



void Cam::print(){
    if(flag_1 == 0){
        Serial.println("No block detected");
    }
    else{
        Serial.print("x: ");
        Serial.print(X);
        Serial.print("  ang: ");
        Serial.print(ang);
        Serial.print(" P : ");
        Serial.print(P);
        Serial.print("  size: ");
        Serial.println(Size);
    }
}