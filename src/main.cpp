#include<Arduino.h>
#include<ac.h>
#include<ball.h>
#include<line.h>
#include<timer.h>
#include<angle.h>
#include<MA.h>
#include<motor_a.h>
#include<US.h>
#include<Cam.h>
#include<OLED_a.h>


#define pi 3.1415926535897932384
#define pi_ 1 / pi 
/*--------------------------------------------------------OLED----------------------------------------------------------------------*/
const int Tact_Switch = 15;  //スイッチのピン番号 
const int Toggle_Switch = 14;  //スイッチのピン番号
int GV = 150;
int RA = 0;
float goDir;
float RA_size;

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
int ball_catch_flag = 0;
int BC;
int get_BC();  //ボール補足センサが反応してるか判定する関数
int ang_180 = 210;
int ang_30 = 80;

//カメラの変数
int cam_flag = 0;
int AC_A = 0;
int AC_B = 0;
int AC_F = 0;

timer cam_T2;

BALL ball;  //ボールのオブジクトだよ(基本的にボールの位置取得は全部ここ)
AC ac;      //姿勢制御のオブジェクトだよ(基本的に姿勢制御は全部ここ)
LINE line;  //ラインのオブジェクトだよ(基本的にラインの判定は全部ここ)
motor_attack MOTOR;
Cam cam;
oled_attack OLED;
float AC_ch();

/*------------------------------------------------------実際に動くやつら-------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);  //シリアルプリントできるよ
  Serial8.begin(57600);
  pinMode(ball_catch,INPUT);
  ac.setup();
  line.setup();
  OLED.OLED();
  GV = OLED.val_max;
  RA = OLED.RA_size;
  A = 10;
}




void loop(){
  double AC_val = 100;  //姿勢制御の最終的な値を入れるグローバル変数
  angle go_ang(0,true);
  float ang_90 = RA;
  int goval = GV;  //動くスピード決定
  int Line_flag = 0;


  if(A == 10){  //情報入手
    ball.getBallposition();              //ボールの位置取得
    Line_flag = line.getLINE_Vec();      //ライン踏んでるか踏んでないかを判定
    ball_catch_flag = get_BC();         //ボール補足センサが反応してるか判定
    AC_val = AC_ch();                    //姿勢制御の値設定
    A = 20;
  }



  if(A == 20){  //進む角度決めるとこ
    if(abs(ball.ang) < 30){
      go_ang = ang_30 / 30 * ball.ang;
    }
    else if(abs(ball.ang) < 90){
      go_ang = ((ang_90 - ang_30) / 60 * (abs(ball.ang) - 30) + ang_30) * ball.ang / abs(ball.ang);
    }
    else{
      go_ang = ((ang_180 - ang_90) / 90 * (abs(ball.ang) - 90) + ang_90) * ball.ang / abs(ball.ang);
    }

    if(ball_catch_flag == 1 || abs(ball.ang) < 10 ){  //ボールを捉えているときは前進するよ
      go_ang = 0;
    }

    A = 30;  //次はライン読むよ!!
  }


  if(A == 30){  //ライン読むところ
    A = 40;
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

    }
    else if(Line_flag == 0){  //ラインを踏んでなかったら
      A_line = 0;
      if(A_line != B_line){  //前回までライン踏んでたら
        B_line = A_line;     //今回はライン踏んでないよ
        if((line_flag_2 == 1  || line_flag == 1)&& cam_flag == 0){
          A = 35;
        }
      }
      line_flag = 0;
      line_flag_2 = 0;
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
    GV = OLED.val_max;
    RA = OLED.RA_size;
  }

  goDir = go_ang.degree;
  RA_size = ang_90;
}


/*----------------------------------------------------------------いろいろ関数-----------------------------------------------------------*/
float AC_ch(){
  float AC_val = 0;
  angle ball_(ball.ang + ac.dir,true);
  ball_.to_range(180,true);
  cam_flag = cam.on;
  AC_A = 0;
  AC_F = 0;

  if(cam_flag == 1){
    if(AC_B == 1){
      if(abs(ball.ang) < 50 && abs(ball_.degree) < 60){
        AC_A = 1;
      }
    }
    else if(AC_B == 0){
      if(abs(ball.ang) < 20 && abs(ball_.degree) < 60){
        AC_A = 1;
      }
    }
  }

  if(AC_A == 0){
    if(AC_A != AC_B){
      AC_B = AC_A;
    }
    AC_val = ac.getAC_val();
  }
  else if(AC_A == 1){
    if(AC_A != AC_B){
      cam_T2.reset();
      AC_B = AC_A;
    }
    if(cam_T2.read_ms() < 500){
      AC_F = 1;
    }
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
  OLED.display.println("C_on");  //この中に変数名を入力
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
  OLED.display.println("RA");  //この中に変数名を入力
  OLED.display.setCursor(30,30);
  OLED.display.println(":");
  OLED.display.setCursor(36,30);
  OLED.display.println(RA_size);    //この中に知りたい変数を入力

  OLED.display.setCursor(0,40); //5列目
  OLED.display.println("");  //この中に変数名を入力
  OLED.display.setCursor(30,40);
  OLED.display.println(":");
  OLED.display.setCursor(36,40);
  OLED.display.println();    //この中に知りたい変数を入力

  OLED.display.setCursor(0,50); //6列目
  OLED.display.println("");  //この中に変数名を入力
  OLED.display.setCursor(30,50);
  OLED.display.println(":");
  OLED.display.setCursor(36,50);
  OLED.display.println();    //この中に知りたい変数を入力
}



int get_BC(){
  BC = analogRead(ball_catch);
  if(BC < 800){
    return 1;
  }
  else{
    return 0;
  }
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