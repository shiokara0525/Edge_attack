#include <Arduino.h>
#include<ball.h>
#include<line.h>
#include<ac.h>
#include<timer.h>


int A = 0;

Ball ball;
LINE line;
AC ac;
timer timer_sawa;

/*---------------------------------------------------------------------------------------------------------------------------------------*/
int line_A = 0; //ラインの変数AB
int line_B = 999;

int line_flag = 0; //ラインをどう踏んでるかを8分割して処理するやつ
int line_flag_2 = 0; //ラインを斜めかまっすぐ踏んでるかのフラグ


double line_target_x[9];
double line_target_y[9];
double line_old_ang;

double line_kp = 0.8;
double line_kkp_x = 0;
double line_kkp_y = 0; 

void moter(double,int,double);
/*---------------------------------------------------------------------------------------------------------------------------------------*/
const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};

const int val_max = 140;

double mSin[4] = {1,1,-1,-1};  //行列式のsinの値
double mCos[4] = {1,-1,-1,1};  //行列式のcosの値
const int Tact_Switch = 15;

void line_reset(int);

void setup() {
  for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);
    pinMode(pah[i],OUTPUT);
  }

  line_target_x[1] = 1;
  line_target_y[1] = 0;
  line_target_x[2] = -1;
  line_target_y[2] = 0;
  line_target_x[3] = 0;
  line_target_y[3] = -1;
  line_target_x[4] = 0;
  line_target_y[4] = 1;
  line_target_x[5] = 1 / sqrt(2);
  line_target_y[5] = -1 / sqrt(2);
  line_target_x[6] = 1 / sqrt(2);
  line_target_y[6] = 1 / sqrt(2);
  line_target_x[7] = -1 / sqrt(2);
  line_target_y[7] = -1 / sqrt(2);
  line_target_x[8] = -1 / sqrt(2);
  line_target_y[8] = 1 / sqrt(2);

  for(int i = 0; i <= 8; i++){
    line_target_x[i] *= 2;
    line_target_y[i] *= 2;
  }

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
        ball.setup();
        ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)
        line.setup();  //ラインとかのセットアップ
        delay(100);
        A = 3;  //メインプログラムいけるよ
      }
    }
    else if(A == 3){
      ball.getBallposition();
      if(digitalRead(Tact_Switch) == LOW){
        A = 4; //スイッチから手が離されるのを待つ
      }
    }
    else if(A == 4){
      if(digitalRead(Tact_Switch) == HIGH){
        A = 10;
      }
    }
  }
  Serial.begin(9600);
}




void loop() {
  int line_on = line.getLINE_Vec();
  double ac_val = ac.getAC_val();
  ball.getBallposition();

  if(line_on == 0){
    line_A = 0;
    if(line_A != line_B){
      line_flag = 0;
      line_B = line_A;
    }
  }
  else if(line_on == 1){
    line_A = 1;
    if(line_A != line_B){
      line_B = line_A;
      line_old_ang = line.Lvec_Dir;

      if(abs(line_old_ang) < 15){
        line_flag = 1;
      }
      else if(165 < abs(line_old_ang)){
        line_flag = 2;
      }
      else if(75 < abs(line_old_ang) && abs(line_old_ang) < 105){
        if(line_old_ang < 0){
          line_flag = 3;
        }
        else{
          line_flag = 4;
        }
      }
      else if(15 < abs(line_old_ang) && abs(line_old_ang) < 75){
        if(line_old_ang < 0){
          line_flag = 5;
        }
        else{
          line_flag = 6;
        }
      }
      else if(105 < abs(line_old_ang) && abs(line_old_ang) < 165){
        if(line_old_ang < 0){
          line_flag = 7;
        }
        else{
          line_flag = 8;
        }
      }
    }  //ここまでが初めて踏んだときに動くとこ(基本はこれ基準で動くぞ！言ったからな！)

    if(line_flag == 1){
      if(15 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 75){
        if(line.Lvec_Dir < 0){
          line_flag = 5;
        }
        else{
          line_flag = 6;
        }
      }
      else if(105 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 165){
        if(line.Lvec_Dir < 0){
          line_flag = 5;
        }
        else{
          line_flag = 6;
        }
      }
    }
    else if(line_flag == 2){
      if(15 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 75){
        if(line.Lvec_Dir < 0){
          line_flag = 7;
        }
        else{
          line_flag = 8;
        }
      }
      else if(105 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 165){
        if(line_old_ang < 0){
          line_flag = 7;
        }
        else{
          line_flag = 8;
        }
      }
    }
    else if(line_flag == 3){
      if(15 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 75){
        line_flag = 5;
      }
      else if(105 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 165){
        line_flag = 7;
      }
    }
    else if(line_flag == 4){
      if(15 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 75){
        line_flag = 6;
      }
      else if(105 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 165){
        line_flag = 8;
      }
    }

    if(line_flag == 5){
      if(abs(line.Lvec_Dir) < 15 || 165 < abs(line.Lvec_Dir)){
        line_flag = 1;
      }
      else if(75 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 105){
        line_flag = 3;
      }
    }
    else if(line_flag == 6){
      if(abs(line.Lvec_Dir) < 15 || 165 < abs(line.Lvec_Dir)){
        line_flag = 1;
      }
      else if(75 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 105){
        line_flag = 4;
      }
    }
    else if(line_flag == 7){
      if(abs(line.Lvec_Dir) < 15 || 165 < abs(line.Lvec_Dir)){
        line_flag = 2;
      }
      else if(75 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 105){
        line_flag = 3;
      }
    }
    else if(line_flag == 8){
      if(abs(line.Lvec_Dir) < 15 || 165 < abs(line.Lvec_Dir)){
        line_flag = 2;
      }
      else if(75 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 105){
        line_flag = 4;
      }
    }

    line_kkp_x = -(line_target_x[line_flag] - line.Lvec_X) * line_kp;
    line_kkp_y = (line_target_y[line_flag] - line.Lvec_Y) * line_kp;
  }

  Serial.print(" ラインのフラグは : ");
  Serial.print(line_flag);
  Serial.print(" ラインの目標値(x) : ");
  Serial.print(line_target_x[line_flag]);
  Serial.print(" (y) : ");
  Serial.print(line_target_y[line_flag]);

  line.print();

  moter(-ball.ang,line_flag,ac_val);
  Serial.println("");
}




void moter(double ang,int flag,double acval){
  double Mval[4] = {0,0,0,0};
  double g = 0;
  double go_x = cos(radians(ang));
  double go_y = sin(radians(ang));
  double val = val_max;

  val -= acval;

  if(flag == 0){
  }
  else if(flag == 1 || flag == 2){
    go_x = line_kkp_x;
  }
  else if(flag == 3 || flag == 4){
    go_y = line_kkp_y;
  }
  else{
    go_x = line_kkp_x;
    go_y = line_kkp_y;
  }

  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * go_x + mCos[i] * go_y;
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i] / g * val + acval;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    if(ac.flag == 1){
      digitalWrite(pah[i],LOW);
      analogWrite(ena[i],0);
    }
    else if(Mval[i] > 0){            //モーターの回転方向が正の時
      digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
      analogWrite(ena[i] , Mval[i]); //モーターの回転速度を設定
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , HIGH);     //モーターの回転方向を負にする
      analogWrite(ena[i] , -Mval[i]);  //モーターの回転速度を設定
    }
  }
}