#include<Arduino.h>
#include<line.h>
#include<ac.h>
#include<ball.h>


const int Tact_Switch = 15; //ロボットのスタートスイッチ


/*-----------------------------------------------------------------------------------------------------------------------------------------*/
LINE line;  //ラインのクラスを宣言
AC ac;
Ball ball;

int Line_on = 0;

double AC_val = 0;

/*-----------------------------------------------------------------------------------------------------------------------------------------*/


void setup(){
  Serial.begin(9600);
  ball.setup();
  line.setup();

  int A = 0;

  pinMode(Tact_Switch,INPUT);

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
      if(digitalRead(Tact_Switch) == HIGH) //ボタンが離されたら、次のステートに行く
      {
        Serial.println(A);
        ac.setup();
        delay(100);
        A = 10; //ラインのベクトルを出すステートに行く
      }
    }
  }
}




void loop(){
}