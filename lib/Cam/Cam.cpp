#include<Cam.h>
#include<ac.h>


Cam::Cam(){
    pixy.init();
}


int Cam::getCamdata(){
    pixy.ccc.getBlocks();
    int size = 0;
    int x = 0;
    int num = 999;
    if(pixy.ccc.numBlocks){
        A = 1;
        if(B != A){
            B = A;
        }
        for(int i = 0; i < pixy.ccc.numBlocks; i++){
            if(pixy.ccc.blocks[i].m_signature == color){
                if(size < pixy.ccc.blocks[i].m_height){
                    size = pixy.ccc.blocks[i].m_height;
                    x = pixy.ccc.blocks[i].m_x;
                    num = i;
                }
            }
        }
        X = x;
        Size = size;
        on = 1;
        if(num == 999){
            on = 0;
        }
    }
    else{
        A = 0;
        if(A != B){
            B = A;
        }
        on = 0;
    }
    return on;
}



void Cam::print(){
    if(on == 0){
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