#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>
#include<MA.h>
#include<moter.h>

/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/


int A = 0;  //どのチャプターに移動するかを決める変数

int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

//上二つの変数を上手い感じにこねくり回して最初に踏んだラインの位置を記録するよ(このやり方は部長に教えてもらったよ)

int line_flag = 0;    //最初にどんな風にラインの判定したか記録
int edge_flag = 0; //ラインの端にいたときにゴールさせる確率を上げるための変数だよ(なんもなかったら0,右の端だったら1,左だったら2)
int side_flag = 0;

const int Tact_Switch = 15;  //スイッチのピン番号 
const double pi = 3.1415926535897932384;  //円周率

void Switch(int);

int val_max = 100;

Ball ball;  //ボールのオブジクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
moter MOTER;
timer Timer;
timer Timer_edge;


/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Wire.begin();  //I2Cできるよ
  ball.setup();  //ボールとかのセットアップ

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
      MOTER.moter_0();
      ball.getBallposition();
      if(ball.far_x != 0 || ball.far_y != 0){
        break;
      }
    }
    A = 30;
  }


  if(A == 20){  //進む角度決めるとこ
    double ang_defference = 75.0 / ball.far;  //どれくらい急に回り込みするか(ボールが近くにあるほど急に回り込みする)
    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    if(ball.ang < 0){  //ここで進む角度決めてるよ!(ボールの角度が負の場合)
      go_ang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : -45) * (0.2 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }
    else{  //(ボールの角度が正の場合)
      go_ang = ball.ang + (abs(ball.ang)<90 ? ball.ang*0.5 : 45) * (0.2 + ang_defference);  //ボールの角度と距離から回り込む角度算出してるよ!
    }

    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    if(abs(go_ang.degrees) < 30){
      goval += 20;
      if(Timer_edge.read_ms() < 2500){
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
      angle linedir(line.Lvec_Dir,true);

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(ここはかなり重要!)
        B_line = A_line;

        line_flag = line.switchLineflag(linedir);

        if(line_flag == 2){
          if(ball.ang < 0){
            Timer_edge.reset();
            edge_flag = 1;
          }
        }
        else if(line_flag == 4){
          if(0 < ball.ang){
            Timer_edge.reset();
            edge_flag = 2;
          }
        }

        if(line.Lrange_num == 1){  //ラインをちょっと踏んでるとき(ここでは緊急性が高くないとする)
          stop_flag = line_flag;   //緊急性高くないし、まともにライン踏んでるから緩めの処理するよ
        }
        else{  //斜めに踏んでるか、またはラインをまたいでるとき(緊急性が高いとするよ,進む角度ごと変えるよ)
          go_ang = line.decideGoang(linedir,line_flag);
        }

        MOTER.moter_0();
        delay(75);
      }
      else{  //連続でライン踏んでるとき
        if(1 < line.Lrange_num){  //ラインをまたいでいたらその真逆に動くよ
          go_ang = line.decideGoang(linedir,line_flag);
          stop_flag = 0;
        }
        else{
          stop_flag = line.switchLineflag(linedir);
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
          go_ang = 180;
          while(abs(ball.ang) < 45){
            double ACval = ac.getAC_val();
            ball.getBallposition();
            
            if(Timer.read_ms() < 200){
              MOTER.moveMoter(go_ang,goval,ACval,0);
            }
            else{
              MOTER.moter_0();
            }

            if(4000 < Timer.read_ms()){
              break;
            }
          }
        }
        else if(line_flag == 3){
          Timer.reset();
          go_ang = 0;
          if(45 < abs(ball.ang) && abs(ball.ang) < 75){
            while(abs(ball.ang) < 90){
              double ACval = ac.getAC_val();
              ball.getBallposition();

              MOTER.moveMoter(go_ang,goval,ACval,0);
              if(400 < Timer.read_ms()){
                break;
              }
            }
          }
        }
      }
      line_flag = 0;
    }
    A = 40;
  }


  if(A == 40){  //最終的に処理するとこ(モーターとかも) 
    MOTER.moveMoter(go_ang,goval,AC_val,stop_flag);  //モーターの処理(ここで渡してるのは進みたい角度,姿勢制御の値,ライン踏んでその時どうするか~ってやつだよ!)
    line.print();
    Serial.print(" ねこ ");
    Serial.print(Line_flag);
    Serial.println();

    if(digitalRead(Tact_Switch) == LOW){
      Switch(2);
    }
    A = 10;
  }
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/




void Switch(int flag){
  int A = 0;
  while(1){
    if(A == 0){
      if(flag == 2){
        if(digitalRead(Tact_Switch) == HIGH){
          delay(100);
          digitalWrite(line.LINE_light,LOW);  //ラインの光止めるよ
          MOTER.moter_0();
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