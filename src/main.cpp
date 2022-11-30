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

double AC_val;  //姿勢制御の最終的な値を入れるグローバル変数
double goang = 0;
double Line_flag = 0;

/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/


void moter(double,double);  //モーター制御関数
double val_max = 200;  //モーターの最大値
int Mang[4] = {45,135,225,315};  //モーターの角度
double mSin[4];  //行列式のsinの値
double mCos[4];  //行列式のcosの値

int A_go = 0;
int B_go = 0;

int go_flag = 0;


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
  double ang_defference = 200 / ball.far;  //ボールが近くにあるほど角度を急に、遠くにあるほど角度を浅くする

  if(A == 10){  //いろんな値を入手するところ
    ball.getBallposition();   //ボールの位置取得
    AC_val = ac.getAC_val();  //姿勢制御用の値を入手
    Line_flag = line.getLINE_Vec();

    A = 20;
  }
  else if(A == 20){  //回り込みのアルゴリズムのとこ
    if(abs(ball.ang) < 30){  //前にボールがあったら
      A_go = 0;
      if(A_go != B_go){
        B_go = A_go;  
      }
      goang = ball.ang;  //普通にボール追いかける

    }
    else if(abs(ball.ang) > 135){  //後ろにボールがあったら
      A_go = 10;
      if(A_go != B_go){  //前Fここに入ってなかったら
        B_go = A_go;  
        if(ball.ang < 0){  //ボールが左にあったら
          go_flag = 0;  //右方向に回り込み
        }
        else{  //ボールが右にあったら
          go_flag = 1;  //左方向に回り込み
        }
      }
      
      if(go_flag == 0){  //右方向に回り込み
        goang = abs(ball.ang) + 2 * ang_defference;  //回り込みの角度を急にする(角度の差分を大きくする)
      }
      else{  //左方向に回り込み
        goang = -abs(ball.ang) - 2 * ang_defference;  //回り込みの角度を急にする(角度の差分を大きくする)
      }

    }
    else{  //ボールが前とも後ろとも言えない横とかにあったら
      A_go = 20; 
      if(A_go != B_go){
        B_go = A_go;  //値更新する
      }
      
      if(ball.ang < 0){  //ボールが左にあったら
        goang = ball.ang - ang_defference;
      }
      else{
        goang = ball.ang + ang_defference;
      }

    }

    A = 30;
  }
  else if(A == 30){
    if(Line_flag != 0){
      if(80 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 100){
        if(goang < 0 && line.Lvec_Dir < 0){
          Line_flag = 1;
        }
        else if(goang > 0 && line.Lvec_Dir > 0){
          Line_flag = 1;
        }
      }
      else if(abs(line.Lvec_Dir) < 10){
        if(abs(goang) < 90){
          Line_flag = 2;
        }
      }
      else if(abs(line.Lvec_Dir) > 170){
        if(abs(goang) > 90){
          Line_flag = 2;
        }
      }
    }
    A = 40;
  }
  else if(A == 40){  //モーターの処理とか値の表示とか
    //moter(goang,AC_val);  //モーター制御
    line.print();
    Serial.print(" ラインのフラグ : ");
    Serial.println(Line_flag);
    A = 10;
  }

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
    if(Line_flag == 0){
      Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    }
    else if(Line_flag == 1){
      Mval[i] = mCos[i] * mval_y;
    }
    else if(Line_flag == 2){
      Mval[i] = -mSin[i] * mval_x;
    }
    
    
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