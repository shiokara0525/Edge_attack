#include<Arduino.h>

/*----------------------------------------------------------------定数------------------------------------------------------------------*/

const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;

const double pi = 3.14159265358979323846264338;

/*------------------------------------------------------------モーター関係---------------------------------------------------------------*/

int Mang[4] = {45,135,225,315};
void moter(double,double);

double Sin[4];
double Cos[4];

/*------------------------------------------------------------動くやつら------------------------------------------------------------------*/




void setup(){
  for(int i = 0; i < 4; i++){
    pinMode(ena[i] , OUTPUT);
    pinMode(pah[i] , OUTPUT);
    Sin[i] = sin(radians(Mang[i]));
    Cos[i] = cos(radians(Mang[i]));
  }

  int A = 0;

  while(A != 10){
    if (A == 0){
      A = 1; //スイッチが押されるのを待つ
    }
    else if(A == 1){
      if(digitalRead(Tact_Switch) == LOW){
        A = 2; //スイッチから手が離されるのを待つ
      }
    }
    else if(A == 2){
      if(digitalRead(Tact_Switch) == HIGH){  //手が離されたらその時点で正面方向決定
        delay(100);
        A = 10;  //メインプログラムいけるよ
      }
    }
  }
}





void loop(){
  double Mgoang = 0;    //進む角度
  double Mgoval = 255;  //進むスピード

  moter(Mgoang,Mgoval);
}





/*------------------------------------------------------------モーターの関数定義-----------------------------------------------------------*/


void moter(double ang,double val){
  double Mval[4] = {0,0,0,0};
  double g = 0;  //一番大きい比の値

  double goval_y = sin(radians(ang));  //進むベクトルのy成分を取り出す
  double goval_x = cos(radians(ang));  //進むベクトルのx成分を取り出す


  for(int i = 0; i < 4; i++){   
    Mval[i] = -Sin[i] * goval_x + Cos[i] * goval_y; //モーターの回転速度を計算(sin)
    
    if(abs(Mval[i]) > g){
      g = abs(Mval[i]);  //一番大きい比の値を取得(これ基準に考える)
    }
  }

  for(int i = 0; i < 4; i++){  //モーターの制御
    Mval[i] = Mval[i] / g * val;  //一番大きい比の値を基準にスピードを調整

    if(Mval[i] > 0){  //モーターの回転方向が正の時
      digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
      analogWrite(ena[i] , Mval[i]);  //モーターの回転速度を設定
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , HIGH);      //モーターの回転方向を負にする
      analogWrite(ena[i] , -Mval[i]);  //モーターの回転速度を設定
    }

  }
}