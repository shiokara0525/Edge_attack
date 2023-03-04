#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>


/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/


int A = 0;  //どのチャプターに移動するかを決める変数

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

//上二つの変数で上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int line_flag = 0;               //最初にどんな風にラインの判定したか記録
double line_switch(int,double);  //ラインを踏んでるときに、ロボットの中心から既にはみ出してしまってるときの対策の関数

const int Tact_Switch = 15;  //スイッチのピン番号 
const double pi = 3.1415926535897932384;  //円周率



Ball ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
timer Timer;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/

const int ena[4] = {28,2,0,4};
const int pah[4] = {29,3,1,5};
void moter(double ang,int val,double ac_val,int stop_flag);  //モーター制御関数
void moter_0();  //モーター止める関数
double val_max = 110;  //モーターの出力の最大値
double mSin[] = {1,1,-1,-1};  //行列式のsinの値
double mCos[] = {1,-1,-1,1};  //行列式のcosの値

#define moter_max 5  //移動平均で使う配列の大きさ
double val_moter[4][moter_max];  //モーターの値を入れる配列(移動平均を使うために二次元にしてるよ)
int count_moter = 0;  //移動平均でリングバッファを使うためのカウンターだよ

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  
  for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);
    pinMode(pah[i],OUTPUT);
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
        ball.setup();
        ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)
        line.setup();  //ラインとかのセットアップ
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
  
  int Line_flag = 0;  //ライン踏んでるか踏んでないか
  int ball_flag = 0;  //ボールがコート上にあるかないか
  int stop_flag = 0;  //ラインをちょっと踏んでるときにどんな動きをするかを決める変数
  int goval = val_max;  //動くスピード決定

  if(A == 10){  //情報入手
    ball_flag = ball.getBallposition();  //ボールの位置取得
    AC_val = ac.getAC_val();             //姿勢制御の値入手
    Line_flag = line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    if(ball_flag == 0){  //ボール見てなかったら
      A = 15;  //止まるとこ
    }
    else{  //ボール見てたら
      A = 20;  //進む角度決めるとこ
    }
  }

  if(A == 15){
    while(1){
      moter_0();
      ball.getBallposition();
      if(ball.far_x != 0 || ball.far_y != 0){
        break;
      }
    }
    A = 30;
  }

  if(A == 20){  //進む角度決めるとこ
    double ang_defference = 80.0 / ball.far;  //どれくらい急に回り込みするか(ボールが近くにあるほど急に回り込みする)

    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    if(ball.ang < 0){  //ここで進む角度決めてるよ!(ボールの角度が負の場合)
      goang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : -45) * (1.0 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }
    else{  //(ボールの角度が正の場合)
      goang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : 45) * (1.0 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }

    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    if(20 < abs(ball.ang) && abs(ball.ang) < 50){  //ボールが斜め前にあったら進む速さちょっと落とすよ
      goval -= 25;
    }

    if(270 < abs(goang)){  //回り込みの差分が大きすぎて逆に前に進むことを防ぐよ
      if(goang < 0){
        goang = -270;
      }
      else{
        goang = 270;
      }
    }
    
    while(180 < abs(goang)){  //角度がの絶対値が180°を超えたらちょっとわかりづらいからわかりやすくするよ
      if(goang < 0){
        goang += 360;
      }
      else{
        goang -= 360;
      }
    }
    A = 30;  //次はライン読むよ!!
  }
  
  if(A == 30){  //ライン読むところ
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;

      float line_dir = (line.Lvec_Dir < 0 ? line.Lvec_Dir + 360 : line.Lvec_Dir);  //ラインの方向を使いやすいように0~360°に変換してるよ 

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(ここはかなり重要!)
        B_line = A_line;

        for(int i = 0; i < 4; i++){  //角度を四つに区分して、それぞれどの区分にいるか判定するよ
          if(i == 0){  //-45°~45°の区分(ここだけ0°をまたいでいるので特別に処理)
            if(315 < line_dir || line_dir < 45){  //-45°~45°にいるとき
              line_flag = i + 1;  //ラインを前のほうで踏んでると判定する
            }
          }
          else{
            if(-45 +(i * 90) < line_dir && line_dir < 45 +(i * 90)){  //それ以外の三つの区分(右、後ろ、左で判定してるよ)
              line_flag = i + 1;
            }
          }
        }

        if(line.Lrange_num == 1){  //ラインをちょっと踏んでるとき(ここでは緊急性が高くないとする)
          stop_flag = line_flag;  //緊急性高くないし、まともにライン踏んでるから緩めの処理するよ
        }
        else{  //斜めに踏んでるか、またはラインをまたいでるとき(緊急性が高いとするよ,進む角度ごと変えるよ)
          for(int i = 0; i < 12; i++){  //角度を12つに区分して、それぞれどの区分にいるか判定する

            if(i == 0){  //-15°~15°の区分(ここだけ0°をまたいでいるので特別に処理)
              if(345 < line_dir || line_dir < 15){
                goang = line_switch(i,line_dir);  //ラインがロボットの中心を通り越すことがあるからそれも考慮してるよ(関数は下にあるよ)
              }
            }
            else{
              if(-15 +(i * 30) < line_dir && line_dir < 15 +(i * 30)){  //時計回りにどの区分にいるか判定してるよ
                goang = line_switch(i,line_dir);
              }
            }
          }
        }
      }
      else{  //連続でライン踏んでるとき
        if(1 < line.Lrange_num){  //ラインをまたいでいたらその真逆に動くよ
          for(int i = 0; i < 12; i++){  //上の繰り返しだよ!!

            if(i == 0){
              if(345 < line_dir || line_dir < 15){
                goang = line_switch(i,line_dir);
              }
            }
            else{
              if(-15 +(i * 30) < line_dir && line_dir < 15 +(i * 30)){
                goang = line_switch(i,line_dir);
              }
            }
          }
          stop_flag = 0;
        }
        else{
          for(int i = 0; i < 4; i++){  //角度を四つに区分して、それぞれどの区分にいるか判定するよ
            if(i == 0){  //-45°~45°の区分(ここだけ0°をまたいでいるので特別に処理)
              if(315 < line_dir || line_dir < 45){  //-45°~45°にいるとき
                stop_flag = i + 1;  //ラインを前のほうで踏んでると判定する
              }
            }
            else{
              if(-45 +(i * 90) < line_dir && line_dir < 45 +(i * 90)){  //それ以外の三つの区分(右、後ろ、左で判定してるよ)
                stop_flag = i + 1;
              }
            }
          }
          line_flag = stop_flag;
        }
      }
      if(line_flag == 0){  //ライン踏んでるけど別に進んでいいよ～って時
        B_line = 0;  //ラインで特に影響受けてないからライン踏んでないのと扱い同じのほうが都合いいよね!
      }
    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;  //今回はライン踏んでないよ
        if(line_flag == 1){
          Timer.reset();
          while(abs(ball.ang) < 45){
            ball.getBallposition();
            moter_0();
            if(2000 < Timer.read_ms()){
              break;
            }
          }
        }
      }
    }
    A = 40;

  }

  if(A == 40){  //最終的に処理するとこ(モーターとかも) 
    moter(goang,goval,AC_val,stop_flag);  //モーターの処理(ここで渡してるのは進みたい角度,姿勢制御の値,ライン踏んでその時どうするか~ってやつだよ!)

    Serial.print(" ラインのフラグ : ");
    Serial.print(line_flag);
    line.print();
    A = 10;
    Serial.println();


    if(digitalRead(Tact_Switch) == LOW){
      A = 50; //スイッチが押されたら
    }

  }
  if(A == 50){
    if(digitalRead(Tact_Switch) == HIGH){
      delay(100);
      A = 60;
      digitalWrite(line.LINE_light,LOW);  //ラインの光止めるよ
      moter_0();
    }
  }
  if(A == 60){
    if(digitalRead(Tact_Switch) == LOW){
      ac.setup_2();  //姿勢制御の値リセットしたよ
      A = 70;
    }
  }
  if(A == 70){
    digitalWrite(line.LINE_light,HIGH);  //ライン付けたよ
    if(digitalRead(Tact_Switch) == HIGH){
      A = 80;  //準備オッケーだよ 
    }
  }
  if(A == 80){
    if(digitalRead(Tact_Switch) == LOW){
      A = 90;  //スイッチはなされたらいよいよスタートだよ
    }
  }
  if(A == 90){
    if(digitalRead(Tact_Switch) == HIGH){
      A = 10;  //スタート!
    }
  }
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/


double line_switch(int i,double ang){  //ラインを踏みこしてるときの処理とか判定とか書いてあるよ
  if(i == 11 || i <= 1){
    if(line_flag == 3){
      return 0.0;
    }
  }
  else if(2 <= i && i <= 4){
    if(line_flag == 4){
      return 90.0;
    }
  }
  else if(5 <= i && i <= 7){
    if(line_flag == 1){
      return 180.0;
    }
  }
  else if(8 <= i && i <= 10){
    if(line_flag == 2){
      return -90.0;
    }
  }

  double goang = (i * 30.0)- 180.0;

  Serial.print(" 踏んだ角度 : ");
  Serial.print(goang);

  return goang;
}




void moter(double ang,int val,double ac_val,int go_flag){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double Mval_n[4] = {0,0,0,0};
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(radians(ang));  //進みたいベクトルのx成分
  double mval_y = sin(radians(ang));  //進みたいベクトルのy成分
  count_moter++;

  float back_val = 2;
  
  max_val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    if(go_flag == 0){
      Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    }
    
    else if(go_flag == 1){  //前のストップかかってたら
      Mval[i] = mCos[i] * mval_y + -mSin[i] * -back_val;
    }
    else if(go_flag == 2){  //右のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * -back_val;
    }
    else if(go_flag == 3){  //後ろのストップかかってたら
      Mval[i] = mCos[i] * mval_y + -mSin[i] * back_val;
    }
    else if(go_flag == 4){  //左のストップかかってたら
      Mval[i] = -mSin[i] * mval_x + mCos[i] * back_val;
    }
    else if(go_flag == 5){
      moter_0();
      return;
    }
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //移動平均求めるゾーンだよ
    Mval[i] /= g;  //モーターの値を制御(常に一番大きい値が1になるようにする)
    Mval_n[i] = Mval[i];  //モーターの値を保存(ライン踏んでるときはこれ使うよ)
    val_moter[i][(count_moter % moter_max)] = Mval[i];  //移動平均を求めるために値を配列に保存
    double valsum_moter = 0;  //移動平均を求めて、その結果の値を保存する変数

    for(int j = 0; j < moter_max; j++){
      valsum_moter += val_moter[i][j];  //過去val_max個の値を足していく
    }

    Mval[i] = valsum_moter / moter_max;  //平均を求めるために割るよ

    if(abs(Mval[i]) > h){  //絶対値が一番高い値だったら
      h = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //モーターの値を計算するところだよ
    if(go_flag == 0){  //ラインの処理してないとき
      Mval[i] = Mval[i] / h * max_val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    }
    else{  //ラインの処理してるとき
      Mval[i] = Mval_n[i] / h * max_val + ac_val;  //移動平均無しバージョンでモーターの値を計算するよ
    }

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




void moter_0(){  //モーターの値を0にする関数
  for(int i = 0; i < 4; i++){
    digitalWrite(pah[i],LOW);
    analogWrite(ena[i],0);
  }
}