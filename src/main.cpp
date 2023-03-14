#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>


/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/


int A = 0;  //どのチャプターに移動するかを決める変数

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int line_flag = 0;    //最初にどんな風にラインの判定したか記録
double edge_flag = 0; //ラインの端にいたときにゴールさせる確率を上げるための変数だよ(なんもなかったら0,右の端だったら1,左だったら2)

const int Tact_Switch = 15;  //スイッチのピン番号 
const double pi = 3.1415926535897932384;  //円周率

void Switch(int);

Ball ball;  //ボールのオブジェクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
timer Timer;


/*--------------------------------------------------------------モーター制御---------------------------------------------------------------*/

const int ena[4] = {0,2,4,28};
const int pah[4] = {1,3,5,29};
void moter(angle ang,int val,double ac_val,int stop_flag);  //モーター制御関数
void moter_0();               //モーター止める関数
double val_max = 125;         //モーターの出力の最大値
double mSin[] = {1,1,-1,-1};  //行列式のsinの値
double mCos[] = {1,-1,-1,1};  //行列式のcosの値

#define moter_max 5              //移動平均で使う配列の大きさ
double val_moter[4][moter_max];  //モーターの値を入れる配列(移動平均を使うために二次元にしてるよ)
int count_moter = 0;             //移動平均でリングバッファを使うためのカウンターだよ

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/




void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ
  
  for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);
    pinMode(pah[i],OUTPUT);
  }  //モーターのピンと行列式に使う定数の設定
  
  Switch(1);
  A = 10;
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  
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
      go_ang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : -45) * (0.5 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }
    else{  //(ボールの角度が正の場合)
      go_ang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : 45) * (0.5 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }

    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    if(abs(go_ang.degrees) < 30){
      goval += 20;
      if(Timer.read_ms() < 2500){
        if(edge_flag == 1){
          go_ang -= 15;
        }
        else if(edge_flag == 2){
          go_ang += 15;
        }
      }
      else{
        edge_flag = 0;
      }
    }

    if(270 < abs(go_ang.degrees)){  //回り込みの差分が大きすぎて逆に前に進むことを防ぐよ
      go_ang = (go_ang.degrees < 0 ? -270 : 270);
    }

    go_ang.to_range(180,true);

    A = 30;  //次はライン読むよ!!
  }


  if(A == 30){  //ライン読むところ
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;
      angle linedir(line.Lvec_Dir,true,360,true);

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(ここはかなり重要!)
        B_line = A_line;
        linedir.to_range(-45,false);

        line_flag = line.switchLineflag(linedir.degrees);

        if(line_flag == 2){
          if(ball.ang < 0){
            Timer.reset();
            edge_flag = 1;
          }
        }
        else if(line_flag == 4){
          if(0 < ball.ang){
            Timer.reset();
            edge_flag = 2;
          }
        }

        if(line.Lrange_num == 1){  //ラインをちょっと踏んでるとき(ここでは緊急性が高くないとする)
          stop_flag = line_flag;   //緊急性高くないし、まともにライン踏んでるから緩めの処理するよ
        }
        else{  //斜めに踏んでるか、またはラインをまたいでるとき(緊急性が高いとするよ,進む角度ごと変えるよ)
          linedir.to_range(-15,false);
          go_ang = line.decideGoang(linedir.degrees,line_flag);
        }

        moter_0();
        delay(50);
      }
      else{  //連続でライン踏んでるとき
        if(1 < line.Lrange_num){  //ラインをまたいでいたらその真逆に動くよ
          linedir.to_range(-15,false);
          go_ang = line.decideGoang(linedir.degrees,line_flag);
          stop_flag = 0;
        }
        else{
          linedir.to_range(-45,false);
          stop_flag = line.switchLineflag(linedir.degrees);
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
            moter_0();
            if(2000 < Timer.read_ms()){
              break;
            }
          }
        }

      }
      line_flag = 0;
    }
    A = 40;
  }


  if(A == 40){  //最終的に処理するとこ(モーターとかも) 
    moter(go_ang,goval,AC_val,stop_flag);  //モーターの処理(ここで渡してるのは進みたい角度,姿勢制御の値,ライン踏んでその時どうするか~ってやつだよ!)

    Serial.println();

    if(digitalRead(Tact_Switch) == LOW){
      Switch(2);
    }
    A = 10;
  }
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/




void moter(angle ang,int val,double ac_val,int go_flag){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double Mval_n[4] = {0,0,0,0};
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(ang.radians);  //進みたいベクトルのx成分
  double mval_y = sin(ang.radians);  //進みたいベクトルのy成分
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
      digitalWrite(pah[i] , HIGH);    //モーターの回転方向を正にする
      analogWrite(ena[i] , Mval[i]); //モーターの回転速度を設定
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , LOW);     //モーターの回転方向を負にする
      analogWrite(ena[i] , -Mval[i]);  //モーターの回転速度を設定
    }
  }
  
  if(ac.flag == 1){  //姿勢制御のせいでモータードライバがストップしちゃいそうだったら
    delay(100);   //ちょっと待つ
    ac.flag = 0;  //姿勢制御のフラグを下ろす
  }
}




void moter_0(){  //モーターの値を0にする関数
  ball.getBallposition();
  for(int i = 0; i < 4; i++){
    digitalWrite(pah[i],LOW);
    analogWrite(ena[i],0);
  }
}




void Switch(int flag){
  int A = 0;
  while(1){
    if(A == 0){
      if(flag == 2){
        if(digitalRead(Tact_Switch) == HIGH){
          delay(100);
          digitalWrite(line.LINE_light,LOW);  //ラインの光止めるよ
          moter_0();
          A = 1;
        }
      }
      else{
        A = 1;
      }
    }

    if(A == 1){
      if(digitalRead(Tact_Switch) == LOW){
        A = 2;
      }
    }

    if(A == 2){
      if(flag == 1){
        ball.setup();
        ac.setup();  //正面方向決定(その他姿勢制御関連のセットアップ)
        line.setup();  //ラインとかのセットアップ
      }
      else{
        ac.setup_2();  //姿勢制御の値リセットしたよ
        digitalWrite(line.LINE_light,HIGH);  //ライン付けたよ
      }
      
      if(digitalRead(Tact_Switch) == HIGH){
        A = 3;  //準備オッケーだよ 
      }
    }

    if(A == 3){
      if(digitalRead(Tact_Switch) == LOW){
        A = 4;  //スイッチはなされたらいよいよスタートだよ
      }
    }
    
    if(A == 4){
      if(digitalRead(Tact_Switch) == HIGH){
        break;
      }
    }
  }
  return;
}