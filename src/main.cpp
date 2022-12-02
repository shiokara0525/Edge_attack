#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>


/*--------------------------------------------------------------定数----------------------------------------------------------------------*/

const int ena[4] = {28,2,0,4};  
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;

const double pi = 3.1415926535897932384;  //円周率

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

int A = 0;  //スイッチを押したらメインプログラムに移動できる変数

Ball ball;  //ボールのクラスのオブジェクトを生成
AC ac;  //姿勢制御のクラスのオブジェクトを生成
LINE line;




/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/




void moter(double,double,int);  //モーター制御関数
double val_max = 200;  //モーターの最大値
int Mang[4] = {45,135,225,315};  //モーターの角度
double mSin[4];  //行列式のsinの値
double mCos[4];  //行列式のcosの値


int A_go = 0;
int B_go = 999;

int A_line = 0;
int B_line = 999;

int A_line_flag = 0;
int B_line_flag = 999;




/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  line.setup();

  for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);  
    pinMode(pah[i],OUTPUT);
    mSin[i] = sin(radians(Mang[i]));
    mCos[i] = cos(radians(Mang[i]));
  }  //モーターのピンと行列式に使う定数の設定

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
        ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)
        delay(100);
        A = 10;  //メインプログラムいけるよ
      }
    }
  }
}




void loop(){
  double AC_val;  //姿勢制御の最終的な値を入れるグローバル変数
  double goang = 0;
  int Line_flag = 0;
  int go_flag = 0;


  ball.getBallposition();
  goang = ball.ang;
  Line_flag = line.getLINE_Vec();
  AC_val = ac.getAC_val();
  

  if(Line_flag == 1){
    A_line = 1;
    if(A_line != B_line){
      B_line = A_line;
      if(70 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 110){
        if(goang < 0 && line.Lvec_Dir < 0){
          go_flag = 1;
          A_line_flag = 1;
        }
        else if(goang > 0 && line.Lvec_Dir > 0){
          go_flag = 2;
          A_line_flag = 2;
        }
      }
      else if(abs(line.Lvec_Dir) < 20){
        if(abs(goang) > 90){
          go_flag = 3;
          A_line_flag = 3;
        }
      }
      else if(abs(line.Lvec_Dir) > 160){
        if(abs(goang) > 90){
          go_flag = 4;
          A_line_flag = 4;
        }
      }
    }
    else{
      go_flag = A_line_flag;
    }
  }
  else if(Line_flag == 0){
    A_line = 0;
    if(A_line != B_line){
      B_line = A_line;
    }
    go_flag = 0;
  }

  moter(goang,AC_val,go_flag);


  line.print();
  Serial.print(" ラインのフラグ : ");
  Serial.print(go_flag);
  Serial.print(" 進む角度 : ");
  Serial.print(goang);
}




/*---------------------------------------------------------------モーター制御関数-----------------------------------------------------------*/




void moter(double ang,double ac_val,int go_flag){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double val = val_max;        //モーターの値の上限値
  double mval_x = cos(radians(ang));  //進みたいベクトルのx成分
  double mval_y = sin(radians(ang));  //進みたいベクトルのy成分

  val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく

  for(int i = 0; i < 4; i++){
    if(go_flag == 0){
      Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    }

    else if(go_flag == 1){
      Mval[i] = -mSin[i] * mval_x + mCos[i] * 1; //モーターの回転速度を計算(行列式で管理)
    }
    else if(go_flag == 2){
      Mval[i] = -mSin[i] * mval_x - mCos[i] * 1;
    }
    else if(go_flag == 3){
      Mval[i] = mCos[i] * mval_y - -mSin[i] + 1.5;
    }
    else if(go_flag == 4){
      Mval[i] = mCos[i] * mval_y + -mSin[i] + 1.5;
    }

    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }

  }

  if(go_flag != 0){
    g = 1;
  }

  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i] / g * val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる) 

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

  if(ac.flag == 1){  //姿勢制御のせいでモータードライバがストップしちゃいそうだったら
    delay(100);   //ちょっと待つ
    ac.flag = 0;  //姿勢制御のフラグを下ろす
  }
}