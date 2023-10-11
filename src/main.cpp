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
#define pi_ 1 / pi 
/*--------------------------------------------------------OLED----------------------------------------------------------------------*/
const int Tact_Switch = 15;  //スイッチのピン番号 
const int Toggle_Switch = 14;  //スイッチのピン番号
int GV = 150;
int V;
int RA = 0;
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

float AC_ch();
//ボールの変数
const int ball_catch = A14;
int ball_catch_flag = 0;
float dif;
float dif_2;
float ang_old = 0;
float b_d;
MA B_D;

//カメラの変数
int cam_flag = 0;
int cam_A = 0;
int cam_B = 999;
int AC_A = 0;
int AC_B = 0;
int C_flag = 0;

timer cam_T;
timer cam_T2;

BALL ball;  //ボールのオブジクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
motor_attack MOTOR;
Cam cam;
oled_attack OLED;
timer Timer;

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Serial8.begin(57600);
  ac.setup();
  line.setup();
  OLED.OLED();
  B_D.setLenth(10);
  B_D.reset();
  GV = OLED.val_max;
  RA = OLED.RA_size;
  A = 10;
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  float ra_size = RA;
  
  int Line_flag = 0;  //ライン踏んでるか踏んでないか
  int goval = GV;  //動くスピード決定


  if(A == 10){  //情報入手
    ball.getBallposition();  //ボールの位置取得
    Line_flag = line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定

    AC_val = AC_ch();
    A = 20;
  }



  if(A == 20){  //進む角度決めるとこ
    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/
    dif_2 = radians(abs(ball.ang)) * pi_;
    dif = (2.5 * abs(sin(radians(ball.ang))) + dif_2);
    go_ang = ball.ang + dif * ra_size *(ball.ang < 0 ? -1 : 1);
    /*-----------------------------------------------------!!!!!!!!!重要!!!!!!!!----------------------------------------------------------*/

    if(270 < abs(go_ang.degree)){  //回り込みの差分が大きすぎて逆に前に進むことを防ぐよ
      go_ang = (go_ang.degree < 0 ? -270 : 270);
    }

    ang_old = ball.ang;
    A = 30;  //次はライン読むよ!!
  }


  if(A == 30){  //ライン読むところ
    if(Line_flag == 1){  //ラインがオンだったら
      A_line = 1;
      timer L;
      angle linedir(line.ang,true);
      angle linedir_2(line.ang + ac.dir,true);
      linedir_2.to_range(180,true);

      if(A_line != B_line){  //前回はライン踏んでなくて今回はライン踏んでるよ～ってとき(ここはかなり重要!)
        B_line = A_line;

        line_flag = line.switchLineflag(linedir);
        line_flag_2 = line.switchLineflag(linedir_2);

        go_ang = line.decideGoang(linedir,line_flag);
        MOTOR.motor_0();
        L.reset();
        while(L.read_ms() < 25 && line.getLINE_Vec() == 1);
      }
      else{  //連続でライン踏んでるとき
        go_ang = line.decideGoang(linedir,line_flag);
      }

      A = 40;
    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;  //今回はライン踏んでないよ
      }
      line_flag = 0;
      line_flag_2 = 0;
      A = 40;
    }
  }


  if(A == 35){  //前にボールがあるとき下がるやつだよ
    timer Timer;
    Timer.reset();

    while(abs(ball.ang) < 45){  //前方向にボールがあるとき
      go_ang = 179.9 - ac.dir;
      ball.getBallposition();
      AC_val = AC_ch();
      if(Timer.read_ms() < 300){  //下がる(0.35秒)
        MOTOR.moveMotor_0(go_ang,goval,AC_val,AC_A);
      }
      else{  //止まるよ
        MOTOR.motor_ac(AC_val);
      }

      if(700 < Timer.read_ms() || line.getLINE_Vec() == 1){
        break;  //1.1秒経つorライン踏んだら抜けるよ
      }

    }
    A = 10;
  }


  if(A == 40){  //最終的に処理するとこ(モーターとかも) 
    MOTOR.moveMotor_0(go_ang,goval,AC_val,AC_A);  //モーターの処理
    OLED_moving();
    A = 10;
  }

  if(digitalRead(Tact_Switch) == LOW){
    MOTOR.motor_0();
    OLED.toogle = digitalRead(Toggle_Switch);
    OLED.OLED();
  }

  goDir = go_ang.degree;
  V = AC_val;
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/
float AC_ch(){
  float AC_val = 0;
  cam_flag = cam.on;
  AC_A = 0;

  if(cam_flag == 1){
    if(abs(ball.ang) < 50){
      AC_A = 1;
    }
  }

  if(AC_A == 0){
    AC_val = ac.getAC_val();
  }
  else if(AC_A == 1){
    AC_val = ac.getCam_val(cam.ang);
  }
  return AC_val;
}




void OLED_moving(){
  //OLEDの初期化
  OLED.display.display();
  OLED.display.clearDisplay();

  //テキストサイズと色の設定
  OLED.display.setTextSize(1);
  OLED.display.setTextColor(WHITE);
  
  OLED.display.setCursor(0,0);  //1列目
  OLED.display.println("goang");  //現在向いてる角度
  OLED.display.setCursor(30,0);
  OLED.display.println(":");
  OLED.display.setCursor(36,0);
  OLED.display.println(goDir);    //現在向いてる角度を表示

  OLED.display.setCursor(0,10);  //2列目
  OLED.display.println("CF");  //この中に変数名を入力
  OLED.display.setCursor(30,10);
  OLED.display.println(":");
  OLED.display.setCursor(36,10);
  OLED.display.println(cam.on);    //この中に知りたい変数を入力a

  OLED.display.setCursor(0,20); //3列目
  OLED.display.println("CA");  //この中に変数名を入力
  OLED.display.setCursor(30,20);
  OLED.display.println(":");
  OLED.display.setCursor(36,20);
  OLED.display.println(cam.ang);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,30); //4列目
  OLED.display.println("L_A");  //この中に変数名を入力
  OLED.display.setCursor(30,30);
  OLED.display.println(":");
  OLED.display.setCursor(36,30);
  OLED.display.println(line.ang);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,40); //5列目
  OLED.display.println("L_on");  //この中に変数名を入力
  OLED.display.setCursor(30,40);
  OLED.display.println(":");
  OLED.display.setCursor(36,40);
  OLED.display.println(line.LINE_on);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,50); //6列目
  OLED.display.println("");  //この中に変数名を入力
  OLED.display.setCursor(30,50);
  OLED.display.println(":");
  OLED.display.setCursor(36,50);
  OLED.display.println();    //この中に知りたい変数を入力
}



void serialEvent1(){
  int a = Serial1.read();
  if(100 < a){
    cam.on = 0;
    cam.ang = 0;
  }
  else{
    cam.on = 1;
    cam.ang = a - 30;
  }
}




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