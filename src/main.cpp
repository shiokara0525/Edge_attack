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
double val_max = 100;  //モーターの最大値
int Mang[4] = {45,135,225,315};  //モーターの角度
double mSin[4];  //行列式のsinの値
double mCos[4];  //行列式のcosの値


int A_go = 0;  //どんな動きしてるか
int B_go = 999;  //前回どんな動きしてたか

int mawarikomi_flag = 0;

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

int line_flag = 0;  //どんな風にラインの判定したか記録


/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  line.setup();  //ラインとかのセットアップ
  
  
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
        A = 3;  //メインプログラムいけるよ
      }
    }
    else if(A == 3){
      if(digitalRead(Tact_Switch) == LOW){
        A = 4; //スイッチから手が離されるのを待つ
      }
    }
    else if(A == 4){
      if(digitalRead(Tact_Switch) == HIGH){
        A = 10; //スイッチから手が離されるのを待つ
      }
    }
  }
}


void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  double goang = 0;  //進みたい角度
  int go_flag = 0;  //回り込みでは回り込みする方向、ラインではラインを踏んでる方向
  double ang_defference = 250 / ball.far;  //どれくらい急に回り込みするか(ボールが近くにあるほど急に回り込みする)
  int Line_flag = 0;  //ライン踏んでるか踏んでないか
  
  
  if(A == 10){  //情報入手
    ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();  //姿勢制御の値入手
    Line_flag = line.getLINE_Vec();  //ライン踏んでるか踏んでないかを判定
    A = 20;
  }
  
  if(A == 20){
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
          mawarikomi_flag = 0;  //右方向に回り込み
        }
        else{  //ボールが右にあったら
          mawarikomi_flag = 1;  //左方向に回り込み
        }
      }
      if(mawarikomi_flag == 0){  //右方向に回り込み
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
        if(ball.ang < 0){
          mawarikomi_flag = 0;
        }
        else{
          mawarikomi_flag = 1;
        }
      }
      if(mawarikomi_flag == 0){  //ボールが左にあったら
        goang = ball.ang - ang_defference;  //左に回り込み
      }
      else{
        goang = ball.ang + ang_defference;  //右に回り込み
      }
    }
    A = 30;
  }
  
  if(A == 30){  //ライン読むところ
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;
      if(A_line != B_line){
        B_line = A_line;
          if(40 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 130){
            if(goang < 0 && line.Lvec_Dir < 0){
              go_flag = 1;
              line_flag = 1;
            }
            else if(goang > 0 && line.Lvec_Dir > 0){
              go_flag = 2;
              line_flag = 2;
            }
          }
        else if(abs(line.Lvec_Dir) < 20){
          if(abs(goang) < 90){
            go_flag = 3;
            line_flag = 3;
          }
        }
        else if(abs(line.Lvec_Dir) > 160){
          if(90 < abs(goang)){
            go_flag = 4;
            line_flag = 4;
          }
        }
        else if(20 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 70){  //前斜め方向にラインあったら
          if(line.Lvec_Dir < 0){  //左前斜め方向にラインあったら
            if(-180 < goang && goang < 90){
              go_flag = 5;
              line_flag = 5;
            }
          }
          else{  //右前斜め方向にラインあったら
            if(-90 < goang && goang < 180){
              go_flag = 6;
              line_flag = 7;
            }
          }
        }
        else if(110 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 160){  //後ろ斜め方向にラインあったら
          if(line.Lvec_Dir < 0){  //左後ろ斜めにラインあったら
            if(goang < 0 || -90 < goang ){
              go_flag = 7;
              line_flag = 7;
            }
          }
          else{  //右後ろ斜めにラインあったら
            if(goang < -90 || 0 < goang){
              go_flag = 8;
              line_flag = 8;
            }
          }
        }
      }
      else{  //連続でライン踏んでたら
        if(30 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 60){  //前斜め方向にラインあったら
          if(line.Lvec_Dir < 0){  //左前斜め方向にラインあったら
            if(-180 < goang && goang < 90){
              go_flag = 5;
            }
          }
          else{  //右前斜め方向にラインあったら
            if(-90 < goang && goang < 180){
              go_flag = 6;
            }
          }
        }
        else if(110 < abs(line.Lvec_Dir) && abs(line.Lvec_Dir) < 160){  //後ろ斜め方向にラインあったら
          if(line.Lvec_Dir < 0){  //左後ろ斜めにラインあったら
            if(goang < 0 || -90 < goang ){
              go_flag = 7;
            }
          }
          else{  //右後ろ斜めにラインあったら
            if(goang < -90 || 0 < goang){
              go_flag = 8;
            }
          }
        }
        else{
          go_flag = line_flag;
        }
      }
    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;  //今回はライン踏んでないよ
      }
      go_flag = 0;  //ライン踏んでない
    }
    A = 40;
  }
  
  if(A == 40){  //最終的に処理するとこ(モーターとかも)
    goang = goang * -1;  //角度を反転させる
    moter(goang,AC_val,go_flag);  //モーターの処理

    A = 10;
    Serial.println("");
  }
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
    
    else if(go_flag == 1){  //左のストップかかってたら
      Mval[i] = -mSin[i] * mval_x - mCos[i] * 4;
    }
    else if(go_flag == 2){  //右のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * 4;
    }
    else if(go_flag == 3){  //前のストップかかってたら
      Mval[i] = mCos[i] * mval_y + mSin[i] * 4;
    }
    else if(go_flag == 4){  //後ろのストップかかってたら
      Mval[i] = mCos[i] * mval_y + mSin[i] * -4;
    }
    else if(go_flag == 5){  //ストップ
      Mval[i] = -mSin[i] * -5 + mCos[i] * -5;
    }
    else if(go_flag == 6){
      Mval[i] = -mSin[i] * -5 + mCos[i] * 5;
    }
    else if(go_flag == 7){
      Mval[i] = -mSin[i] * 4 + mCos[i] * -4;
    }
    else if(go_flag == 8){
      Mval[i] = -mSin[i] * 4 + mCos[i] * 4;
    }
    else if(go_flag == 9){
      Mval[i] = 0;
    }
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }
  
  if(go_flag != 0){  //ラインから逃げる感じの雰囲気だったら
    g = 2;  //180°単位の移動方向の変化があるから出力控え目
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