#include<Arduino.h>
#include<Wire.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>
#include<MA.h>
#include<motor_a.h>
#include<US.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Encoder.h>
#include<Cam.h>
#include<OLED_a.h>


#define pi 3.1415926535897932384
/*--------------------------------------------------------OLED----------------------------------------------------------------------*/
const int Tact_Switch = 15;  //スイッチのピン番号 
const int Toggle_Switch = 14;  //スイッチのピン番号
int val_max = 150;
int RA_size = 0;
float goDir;
int BF_flag = 0;

void OLED_moving();
/*--------------------------------------------------------いろいろ変数----------------------------------------------------------------------*/

int A = 0;  //どのチャプターに移動するかを決める変数

//ラインの変数
int A_line = 0;  //ライン踏んでるか踏んでないか
int B_line = 999;  //前回踏んでるか踏んでないか

int line_flag = 0;    //最初にどんな風にラインの判定したか記録
int line_flag_2 = 0;

//ボールの変数
const int ball_catch = A14;
float ball_Far = 0;
int ball_catch_flag = 0;

//カメラの変数
int cam_flag = 0;

BALL ball;  //ボールのオブジクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
motor_attack MOTOR;
Cam cam;
oled_attack OLED;

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Serial8.begin(57600);
  ac.setup();
  line.setup();
  OLED.OLED();
  A = 10;
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  float ra_size = RA_size;
  
  int Line_flag = 0;  //ライン踏んでるか踏んでないか
  int goval = val_max;  //動くスピード決定


  if(A == 10){  //情報入手
    ball.getBallposition();  //ボールの位置取得
    Line_flag = line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    if(abs(ball.ang) < 20){
      if(analogRead(ball_catch) < 800){
        ball_catch_flag = 1;
      }
      else{
        ball_catch_flag = 0;
      }
    }
    ball_catch_flag = 0;

    if(Line_flag == 1){
      ball_catch_flag = 2;
    }
    cam_flag = cam.getCamdata(ac.getnowdir(),ball.ang,ball_catch_flag);  //姿勢制御の値入手
    A = 20;
  }


  if(A == 15){
    while(1){
      MOTOR.motor_0();
      ball.getBallposition();
    }
    A = 30;
  }


  if(A == 20){  //進む角度決めるとこ
    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/
    float ball_far = ball.far;
    if(ball_far < 40){
      ball_far = 40;
    }
    else if(ball_far < 65){
      ball_far = 55;
    }
    else if(ball_far < 80){
      ball_far = 80;
    }
    else{
      ball_far = 100;
    }

    if(cam.flag_1 == 1){
      goval = 80;
      ball_far = 40;
      ra_size += 15;
    }
    else if(cam.flag_1 == 2 && 30 < abs(ball.ang)){
      goval = 140;
      ra_size += 5;
      ball_far = 80;
    }
    

    ball_Far = ball_far;
    if(ball.ang < 0){
      go_ang = ball.ang + (ra_size / ball_far) * (90 < abs(ball.ang) ? -90 : ball.ang);
    }
    else{
      go_ang = ball.ang + (ra_size / ball_far) * (90 < ball.ang ? 90 : ball.ang);
    }
    // if(ball_catch_flag == 1){
    //   go_ang = 0;
    // }
    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    
    if(270 < abs(go_ang.degree)){  //回り込みの差分が大きすぎて逆に前に進むことを防ぐよ
      go_ang = (go_ang.degree < 0 ? -270 : 270);
    }

    go_ang.to_range(180,true);
    if(ball_catch_flag == 1){
      go_ang = 0;
    }

    if(abs(ball.ang) < 25){
      goval += 20;
    }

    A = 30;  //次はライン読むよ!!
  }


  if(A == 30){  //ライン読むところ
    A = 40;
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;
      angle linedir(line.Lvec_Dir,true);
      angle linedir_2(line.Lvec_Dir + ac.dir,true);
      linedir_2.to_range(180,true);

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(ここはかなり重要!)
        B_line = A_line;

        line_flag = line.switchLineflag(linedir);
        line_flag_2 = line.switchLineflag(linedir_2);

        go_ang = line.decideGoang(linedir,line_flag);
        if(3 <= line.Lrange_num){
          if(abs(ball.ang) < 90){
            go_ang = 179.9;
          }
          else{
            go_ang = 0;
          }
        }
        MOTOR.motor_0();
        delay(75);
      }
      else{  //連続でライン踏んでるとき
        go_ang = line.decideGoang(linedir,line_flag);
      }

      if((120 < abs(cam.X - 150) || cam.flag_2 == 0 || 40 < abs(ac.dir) || 3 <= line.Lrange_num) && line_flag_2 == 1){
        A = 35;
      }

      // if(cam.Size < 12 && abs(ac.dir) < 10){
      //   if(line_flag == 2){
      //     if((90 < cam.X && cam.X < 110) && (60 < ball.ang && ball.ang < 90)){
      //       A = 36;
      //     }
      //   }
      //   else if(line_flag == 4){
      //     if((190 < cam.X && cam.X < 205) && (-90 < ball.ang && ball.ang < -60)){
      //       A = 36;
      //     }
      //   }
      //   else if(line_flag == 3){
      //     if((cam.X < 100 || 200 < cam.X) && (60 < abs(ball.ang) && abs(ball.ang) < 90)){
      //       A = 36;
      //     }
      //     if((100 < cam.X && cam.X < 200) && (60 < abs(ball.ang) && abs(ball.ang) < 120)){
      //       A = 37;
      //     }
      //   }
      // }

    
      if(line_flag == 0){  //ライン踏んでるけど別に進んでいいよ～って時
        B_line = 0;  //ラインで特に影響受けてないからライン踏んでないのと扱い同じのほうが都合いいよね!
      }
    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;  //今回はライン踏んでないよ
      }
      if(A != 36){
        line_flag = 0;
      }
    }
  }




  if(A == 35){  //前にボールがあるとき下がるやつだよ
    timer Timer;
    Timer.reset();
    if(BF_flag == 0){
      go_ang = 179.9;
    }
    else{
      go_ang = 0;
    }

    while(1){  //前方向にボールがあるとき
      if(analogRead(ball_catch) < 800){
        ball_catch_flag = 1;
      }
      else{
        ball_catch_flag = 0;
      }
      go_ang = 179.9 - ac.dir;
      cam.getCamdata(ac.getnowdir(),ball.ang,1);
      ball.getBallposition();
      if(Timer.read_ms() < 250){  //下がる(0.35秒)
        MOTOR.moveMoter_L(go_ang,goval,cam.P,line);
      }
      else{  //止まるよ
        MOTOR.motor_ac(cam.P);
        if(BF_flag == 1){
          break;
        }
      }

      if(BF_flag == 0){
        if(700 < Timer.read_ms() || line.getLINE_Vec() == 1){
          break;  //1.1秒経つorライン踏んだら抜けるよ
        }
      }

    }
    A = 10;
  }



  if(A == 36){
    go_ang = 0;
    while(abs(ball.ang) < 90){
      ball.getBallposition();
      AC_val = ac.getAC_val();
      MOTOR.moveMoter_0(go_ang,120,AC_val);
      OLED_moving();
    }
    A = 10;
  }


  if(A == 37){
    if(ball.ang < 0){
      go_ang = -90;
    }
    else{
      go_ang = 90;
    }

    while(30 < abs(ball.ang)){
      ball.getBallposition();
      AC_val = ac.getAC_val();
      MOTOR.moveMoter_0(go_ang,120,AC_val);
      OLED_moving(); 
    }
    A = 10;
  }


  if(A == 40){  //最終的に処理するとこ(モーターとかも) 
    MOTOR.moveMoter_0(go_ang,goval,cam.P);  //モーターの処理
    A = 10;
  }

  if(digitalRead(Tact_Switch) == LOW){
    MOTOR.motor_0();
    OLED.toogle = digitalRead(Toggle_Switch);
    OLED.OLED();
  }

  goDir = go_ang.degree;
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/
void serialEvent8(){
  int n;
  int x,y;
  word revBuf_word[6];
  byte revBuf_byte[6];
  //受信データ数が、一定時間同じであれば、受信完了としてデータ読み出しを開始処理を開始する。
  //受信データあり ※6バイト以上になるまでまつ
  if(Serial8.available()>= 6){
    //---------------------------
    //受信データをバッファに格納
    //---------------------------
    n = 0;
    while(Serial8.available()>0 ){ //受信データがなくなるまで読み続ける
      //6バイト目まではデータを格納、それ以上は不要なデータであるため捨てる。
      if(n < 6){
        revBuf_byte[n] = Serial8.read();   //revBuf_byte[n] = Serial2.read();
      }
      else{
        Serial8.read(); //Serial2.read();  //読みだすのみで格納しない。
      }
      n++;
    }
    //---------------------------
    //データの中身を確認
    //---------------------------
    //データの先頭、終了コードあることを確認
    if((revBuf_byte[0] == 0xFF ) && ( revBuf_byte[5] == 0xAA )){
    //いったんWORD型（16bitデータ）としてから、int16_tとする。
      revBuf_word[0] = (uint16_t(revBuf_byte[1])<< 8);//上位8ビットをbyteから、Wordに型変換して格納　上位桁にするため8ビットシフト
      revBuf_word[1] = uint16_t(revBuf_byte[2]);//下位8ビットをbyteから、Wordに型変換して格納      
      x = int16_t(revBuf_word[0]|revBuf_word[1]);//上位8ビット、下位ビットを合成（ビットのORを取ることで格納する。）
      // ※int ではなく　int16_t　にすることが必要。intだけだと、32ビットのintと解釈されマイナス値がマイナスとみなされなくなる、int16_tは、16ビット指定の整数型変換
      revBuf_word[2] = (uint16_t(revBuf_byte[3])<< 8);//上位8ビットをbyteから、Wordに型変換して格納　上位桁にするため8ビットシフト
      revBuf_word[3] = uint16_t(revBuf_byte[4]);//下位8ビットをbyteから、Wordに型変換して格納      
      y = int16_t(revBuf_word[2]|revBuf_word[3]);//上位8ビット、下位ビットを合成（ビットのORを取ることで格納する。）
      // ※int ではなく　int16_t　にすることが必要。intだけだと、32ビットのintと解釈されマイナス値がマイナスとみなされなくなる、int16_tは、16ビット指定の整数型変換
      
      x = ball.ball_x.demandAve(x);
      y = ball.ball_y.demandAve(y);
    }
    else{
      // printf("ERR_REV");
    }
  }
}



void OLED_moving(){
  //OLEDの初期化
  OLED.display.display();
  OLED.display.clearDisplay();

  //テキストサイズと色の設定
  OLED.display.setTextSize(1);
  OLED.display.setTextColor(WHITE);
  
  OLED.display.setCursor(0,0);  //1列目
  OLED.display.println("Bang");  //現在向いてる角度
  OLED.display.setCursor(30,0);
  OLED.display.println(":");
  OLED.display.setCursor(36,0);
  OLED.display.println(ball.ang);    //現在向いてる角度を表示

  OLED.display.setCursor(0,10);  //2列目
  OLED.display.println("goang");  //この中に変数名を入力
  OLED.display.setCursor(30,10);
  OLED.display.println(":");
  OLED.display.setCursor(36,10);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,20); //3列目
  OLED.display.println("C_x");  //この中に変数名を入力
  OLED.display.setCursor(30,20);
  OLED.display.println(":");
  OLED.display.setCursor(36,20);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,30); //4列目
  OLED.display.println("bcf");  //この中に変数名を入力
  OLED.display.setCursor(30,30);
  OLED.display.println(":");
  OLED.display.setCursor(36,30);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,40); //5列目
  OLED.display.println("LF");  //この中に変数名を入力
  OLED.display.setCursor(30,40);
  OLED.display.println(":");
  OLED.display.setCursor(36,40);
  OLED.display.println(line.LINE_on);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,50); //6列目
  OLED.display.println("A");  //この中に変数名を入力
  OLED.display.setCursor(30,50);
  OLED.display.println(":");
  OLED.display.setCursor(36,50);
  OLED.display.println(A);    //この中に知りたい変数を入力
}