#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>


/*--------------------------------------------------------------定数----------------------------------------------------------------------*/

const int ena[4] = {28,2,0,4};  
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;

const double pi = 3.1415926535897932384;  //円周率

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

int A = 0;  //スイッチを押したらメインプログラムに移動できる変数

Ball ball;  //ボールのクラスのオブジェクトを生成
AC ac;  //姿勢制御のクラスのオブジェクトを生成

double AC_val;  //姿勢制御の最終的な値を入れるグローバル変数

/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


void moter(double,double);  //モーター制御関数
double val_max = 200;  //モーターの最大値
int Mang[4] = {45,135,225,315};  //モーターの角度
double mSin[4];  //行列式のsinの値
double mCos[4];  //行列式のcosの値


/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ

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
  ball.getBallposition();   //ボールの位置取得
  AC_val = ac.getAC_val();  //姿勢制御用の値を入手
  
  moter(ball.ang,AC_val);  //進みたい方向、姿勢制御用の値をアウトプットしてモーターに渡す
  ac.print();
}




/*---------------------------------------------------------------モーター制御関数-----------------------------------------------------------*/




void moter(double ang,double ac_val){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double val = val_max;        //モーターの値の上限値
  double mval_x = cos(radians(ang));  //進みたいベクトルのx成分
  double mval_y = sin(radians(ang));  //進みたいベクトルのy成分

  val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく

  for(int i = 0; i < 4; i++){   
    Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
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