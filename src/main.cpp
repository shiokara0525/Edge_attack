#include<Arduino.h>
#include<line.h>

LINE line;  //ラインのクラスを宣言
const int Tact_Switch = 15; //ロボットのスタートスイッチ



void setup(){
  Serial.begin(9600);
  line.setup();  //ラインのセットアップ

  int A = 0;

  pinMode(Tact_Switch,INPUT);

  if (A == 0){
    A = 1; //スイッチが押されるのを待つ
  }
  else if(A == 1){
    if(digitalRead(Tact_Switch) == LOW) //ボタンを押されたら、離されるのを待つステートに行く
    {
      A = 2; //スイッチから手が離されるのを待つ
    }
  }
  else if(A == 2){
    if(digitalRead(Tact_Switch) == HIGH) //ボタンが離されたら、次のステートに行く
    {
      delay(100);
      A = 3; //ラインのベクトルを出すステートに行く
    }
  }
}



void loop(){
  line.getLINE_Vec();  //ラインのベクトルを取得
  line.print();  //ラインのベクトルを表示
}