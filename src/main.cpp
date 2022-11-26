#include<Arduino.h>
#include<Wire.h>
#include<Adafruit_Sensor.h>
#include<Adafruit_BNO055.h>
#include<Adafruit_SPIDevice.h>


/*--------------------------------------------------------------定数----------------------------------------------------------------------*/

const int ball_sen[16] ={
  9,10,11,12,13,34,35,36,37,38,39,40,41,6,7,8};
const int ena[4] = {28,2,0,4};  
const int pah[4] = {29,3,1,5};
const int Tact_Switch = 15;

const double pi = 3.1415926535897932384;  //円周率


/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

int A = 0;  //スイッチを押したらメインプログラムに移動できる変数

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
  double Sin[16]; //sinの値(22.5°ずつ)
  double Cos[16]; //cosの値(22.5°ずつ)
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
  double nowTime = 0;  //関数で見た時の時間
  double time_old = 0; //1F前の時間

  double val = 0;  //姿勢制御の値
  double val_old = 0;  //1F前の姿勢制御の値

  double dir = 0;  //現Fの方向
  double dir_old = 0;  //前Fの方向
  sensors_event_t event;  //ジャイロのいろんな値入れるやつ
  Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
};
AC ac;

double AC_val = 0;  //姿勢制御の値(グローバル変数)



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
}




/*-----------------------------------------------------------姿勢制御用の関数---------------------------------------------------------------*/




double AC::getAC_val(){  //姿勢制御の値返す関数
  double kkp = 0;  //比例制御の値
  double kkd = 0;  //積分制御の値
    

  bno.getEvent(&event);  //方向チェック
  
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




void AC::print(){  //現在の角度、正面方向、姿勢制御の最終的な値を表示
  Serial.print(" 角度 : ");
  Serial.print(dir);
  Serial.print(" 正面方向 : ");
  Serial.print(dir_target);
  Serial.print(" 最終的に出たやつ : ");
  Serial.print(val);
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




void Ball::getBallposition(){  //ボールの位置を極座標系で取得
  double Bfar = 0;  //グローバル変数に戻す前の変数(直接代入するのはは何となく不安)
  double Bang = 0;  //グローバル変数に戻す前の変数
  int Bval[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //ボールの値

  double Bfar_x = 0; //ボールの距離のx成分
  double Bfar_y = 0; //ボールの距離のy成分

  int low_cou = 0;    //一回ボールの値を集計して値がch_num以下だったセンサーの数
  double low_all = 0; //最新100回のボールの値を集計して値がch_num以下だったセンサーの数


  for(int ch_cou = 0; ch_cou < ch_num; ch_cou++){   //ch_num回センサーの値を取得
    for(int sen_num = 0; sen_num < 16; sen_num++){  //16個のセンサーの値を取得

      if(digitalReadFast(ball_sen[sen_num]) == 0){
        Bval[sen_num]++;    //センサーの値が0だったらBvalに1を足す(遠くにあるほど値が大きくなる)
      }

    }
  }

  for(int i = 0; i < 16; i++){   //値を集計するところ
    Bfar_x += Bval[i] * Cos[i];  //ボールの距離のx成分を抽出
    Bfar_y += Bval[i] * Sin[i];  //ボールの距離のy成分を抽出

    if(Bval[i] < sen_lowest){
      low_cou++;  //値がsen_lowest以下だったセンサーの数をカウント
    }
  }

  low_acc[cou % MAX] = low_cou;  //最新の値を配列に入れる(リングバッファ使ってる)
  cou++;  //この関数の呼び出し回数をカウント

  for(int i = 0; i < 100; i++){
    low_all += low_acc[i];  //値がsen_lowest以下だったセンサーの数を合計
  }

  Bfar = low_all / (cou < 100 ? cou : 100);  //ボールの距離を計算
  Bang = atan2(Bfar_y,Bfar_x) * 180 / pi;    //ボールの角度を計算(atan2はラジアンで返すので角度に変換)  

  ang = Bang;
  far = Bfar;
}




void Ball::print(){  //ボールの位置を表示
  Serial.print(" ボールの距離 : ");
  Serial.print(far);
  Serial.print(" ボールの角度 : ");
  Serial.println(ang);
}




void Ball::setup(){  //ボール関連のセットアップ
  for(int i = 0; i < 16; i++){
    pinMode(ball_sen[i],INPUT);
    Cos[i] = cos(radians(i * 22.5));  //cosの22.5度ごとの値を配列に入れる
    Sin[i] = sin(radians(i * 22.5));  //sinの22.5度ごとの値を配列に入れる
  }
  for(int i = 0; i < 100; i++){
    low_acc[i] = 0;  //配列の中の値を0にする
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