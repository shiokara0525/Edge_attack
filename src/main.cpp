#include<Arduino.h>
#include<Wire.h>
#include<Adafruit_Sensor.h>
#include<Adafruit_BNO055.h>
#include<Adafruit_SPIDevice.h>

/*--------------------------------------------------------------定数----------------------------------------------------------------------*/

const int ball_sen[16] ={
  9,10,11,12,13,34,35,36,37,38,39,40,41,6,7,8};  //ボールセンサーのピン番号
const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};  
const int Tact_Switch = 15;  //スイッチのピン番号

const double pi = 3.14159265358979323846264338;  //円周率

/*------------------------------------------------------------モーター関係---------------------------------------------------------------*/

int Mang[4] = {45,135,225,315};  //モーターのついてる角度
void moter(double,double);  //モーターの処理関数

double Mval_max = 200;  //モーターの最大出力値

double Sin[4];  //行列式用のsin
double Cos[4];  //行列式用のcos

/*----------------------------------------------------------ボール------------------------------------------------------------------------*/


const int ch_num = 1000; //センサーの値取る回数
const int sen_lowest = 200; //センサーがボールを見てないと判断する値

#define MAX 100

class Ball{
public:
  double far;  //ボールまでの距離
  double ang;  //ボールまでの角度
  void getBallposition();  //ボールの位置を取得
  void print();  //ボールの距離と角度を表示
  void setup();  //セットアップ

private:
  int cou = 0;  //ボールを見た回数(getBallpositionに入った回数をカウントするやつ)
  double low_acc[MAX];  //ボールまでの距離(最新100回分をはかるように、円環バッファを使う)
  
  double bSin[16]; //sinの値(22.5°ずつ)
  double bCos[16]; //cosの値(22.5°ずつ)
};
Ball ball;  //実体を生成


/*--------------------------------------------------------------姿勢制御-------------------------------------------------------------------*/


const float kp = 3;  //比例制御の比例定数
const float kd = 5;  //微分制御の定数

double dir_target;  //目標の方向(正面方向)

class AC{
public:
  double getAC_val(); //姿勢制御用の値返す関数
  int flag = 0;  //モーターが突然反転しないようにするやつ
  void print();  //姿勢制御関連のやつを表示
  void setup();  //姿勢制御のセットアップ

private:
  double kkp = 0;  //比例制御の値
  double kkd = 0;  //積分制御の値

  double nowTime = 0;  //関数で見た時の時間
  double time_old = 0; //1F前の時間

  double val = 0;  //姿勢制御の値
  double val_old = 0;  //1F前の姿勢制御の値

  double dir = 0;  //現Fの方向
  double dir_old = 0;  //前Fの方向
  sensors_event_t event;
  Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
};
AC ac;

double AC_val = 0;  //姿勢制御の値(グローバル変数)




/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールのセットアップ
  
  for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);  
    pinMode(pah[i],OUTPUT);
    Sin[i] = sin(radians(Mang[i]));  //モーター制御用の変数
    Cos[i] = cos(radians(Mang[i]));  //モーター制御用の変数
  }  //モーターのピンの設定

  int A = 0;  //スイッチを押したかどうかの判定用の変数

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
  
  moter(ball.ang,AC_val);  //進みたい方向、姿勢制御用の値をモーターに渡す
}




/*-----------------------------------------------------------姿勢制御用の関数---------------------------------------------------------------*/




double AC::getAC_val(){  //姿勢制御の値返す関数

  bno.getEvent(&event);  
  
  dir = event.orientation.x - dir_target;  //現在の方向を取得
  nowTime = millis();  //現在の時間を取得
  
  if(dir > 180){
    dir -= 360;  //方向を0~360から-180~180に変換
  }

  kkp = -dir;  //比例制御の値を計算
  kkd = (dir - dir_old) / (nowTime - time_old);  //微分制御の値を計算
  
  val = kkp * kp + kkd * kd;  //最終的に返す値を計算

  if(abs(dir - dir_old) > 350){
    ac.flag = 1;  //モーターが急に反転してストップするのを防止するフラグ
  }
  

  dir_old = dir;  //前Fの方向を更新
  time_old = nowTime;  //前Fの時間を更新

  return val;  //値返す
}




void AC::print(){  //現在の角度、正面方向、姿勢制御の値を表示
  Serial.print(" 角度 : ");
  Serial.print(dir);
  Serial.print(" 正面方向 : ");
  Serial.print(dir_target);
  Serial.print(" kkp : ");
  Serial.print(kkp * kp);  
  Serial.print(" kkd : ");
  Serial.println(kkd * kd);
}




void AC::setup(){  //セットアップ
  bno.begin();
  bno.getEvent(&event);  //方向入手

  if(event.orientation.x > 180){
    event.orientation.x -= 360;  //方向を0~360から-180~180に変換
  }

  dir_target = event.orientation.x;  //正面方向決定
}




/*------------------------------------------------------------------ボールの関数-----------------------------------------------------------*/




void Ball::getBallposition(){

  double Bfar = 0;  //グローバル変数に戻す前の変数(直接代入するのはは何となく不安)
  double Bang = 0;  //グローバル変数に戻す前の変数
  int Bval[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //ボールの値

  double Bfar_x = 0; //ボールの距離のx成分
  double Bfar_y = 0; //ボールの距離のy成分
  
  int low_cou = 0; //一回ボールの値を集計して値がch_num以下だったセンサーの数
  double low_all = 0; //最新100回のボールの値を集計して値がch_num以下だったセンサーの数


  for(int ch_cou = 0; ch_cou < ch_num; ch_cou++){  //ch_num回センサーの値を取得
    for(int sen_num = 0; sen_num < 16; sen_num++){  //16個のセンサーの値を取得

      if(digitalReadFast(ball_sen[sen_num]) == 0){
        Bval[sen_num]++;    //センサーの値が0だったらBvalに1を足す(遠くにあるほど値が大きくなる)
      }

    }
  }

  for(int i = 0; i < 16; i++){   //値を集計するところ
    Bfar_x += Bval[i] * bCos[i];  //ボールの距離のx成分を抽出
    Bfar_y += Bval[i] * bSin[i];  //ボールの距離のy成分を抽出

    if(Bval[i] < sen_lowest){
      low_cou++;  //値がsen_lowest以下だったセンサーの数をカウント
    }
  }

  low_acc[cou % MAX] = low_cou;
  cou++;

  for(int i = 0; i < 100; i++){
    low_all += low_acc[i];  //値がsen_lowest以下だったセンサーの数を合計
  }

  Bfar = low_all / (cou < 100 ? cou : 100);  //ボールの距離を計算
  Bang = atan2(Bfar_y,Bfar_x) * 180 / pi;  //ボールの角度を計算(atan2はラジアンで返すので角度に変換)  

  ang = Bang;
  far = Bfar;
}




void Ball::print(){
  Serial.print(" ボールの距離 : ");
  Serial.print(far);
  Serial.print(" ボールの角度 : ");
  Serial.println(ang);
}




void Ball::setup(){
  for(int i = 0; i < 16; i++){
    pinMode(ball_sen[i],INPUT);
    bCos[i] = cos(radians(i * 22.5));
    bSin[i] = sin(radians(i * 22.5));
  }
  for(int i = 0; i < 100; i++){
    low_acc[i] = 0;
  }
}



/*---------------------------------------------------------------モーター制御関数-----------------------------------------------------------*/




void moter(double ang,double ac_val){
  double g = 0;  //一番高いモーターの値
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double goval_y = sin(radians(ang));  //進みたいベクトルのx成分
  double goval_x = cos(radians(ang));  //進みたいベクトルのy成分
  double val = Mval_max;  //モーターの値の最大値
  

  val -= ac_val;  //いい感じに姿勢制御できるようにモーターの値を調整する(姿勢制御の値を引く)

  for(int i = 0; i < 4; i++){   
    Mval[i] = -Sin[i] * goval_x + Cos[i] * goval_y; //モーターの回転速度を計算(sin)
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);  //一番大きい値を代入
    }
  }
  
  for(int i = 0; i < 4; i++){
    Mval[i] = Mval[i] / g * val + ac_val;  //一番大きい値を255で割ってモーターの値を調整

    if(ac.flag == 1){
      digitalWrite(pah[i],LOW);
      analogWrite(ena[i],0);
    }
    else if(Mval[i] > 0){  //モーターの回転方向が正の時
      digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
      analogWrite(ena[i] , Mval[i]);  //モーターの回転速度を設定
    } 
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , HIGH);      //モーターの回転方向を負にする
      analogWrite(ena[i] , -Mval[i]);  //モーターの回転速度を設定
    }
  }

  if(ac.flag == 1){
    delay(100);
    Serial.println(ac.flag);
    ac.flag = 0;
  }
}