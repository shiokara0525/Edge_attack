#include <Arduino.h>
#include <MA.h>
#include<timer.h>
#include<line.h>
#include<ball.h>
#include<ac.h>
#include<moter.h>
const int pingPin = 32;
const int Tact_Switch = 15;
MA US;
timer Timer;
LINE line;
moter MOTER;
AC ac;
Ball ball;


int readUS();
void Switch(int);

void setup(){
  pinMode(pingPin, OUTPUT);
  Serial.begin(9600);
  US.setLenth(10);
  Switch(1);
}


void loop(){
  float length;
  double Long;
  Timer.reset();
  length = US.demandAve(readUS());
  Long = Timer.read_ms();
  Serial.print("  ");
  Serial.print(length);
  Serial.print("cm  ");
  Serial.print(Long);
  Serial.print("ms");
  Serial.println();

  if(digitalReadFast(Tact_Switch) == LOW){
    Switch(2);
  }
}

int readUS(){
  unsigned long duration;
  int cm;
  //ピンをOUTPUTに設定（パルス送信のため）
  pinMode(pingPin, OUTPUT);
  //LOWパルスを送信
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);  
  //HIGHパルスを送信
  digitalWrite(pingPin, HIGH);  
  //5uSパルスを送信してPingSensorを起動
  delayMicroseconds(5); 
  digitalWrite(pingPin, LOW); 
  
  //入力パルスを読み取るためにデジタルピンをINPUTに変更（シグナルピンを入力に切り替え）
  pinMode(pingPin, INPUT);
  //入力パルスの長さを測定
  duration = pulseIn(pingPin, HIGH,6000);
  Serial.print(duration);

  //パルスの長さを半分に分
  duration=duration/2;
  //cmに変換
  cm = int(duration/29); 
  if(cm == 0){
    cm = 100;
  }

  delayMicroseconds(100);
  return cm;
}




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