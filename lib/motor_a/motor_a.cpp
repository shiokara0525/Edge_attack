#include<motor_a.h>



motor_attack::motor_attack(){
	for(int i = 0; i < 4; i++){
    pinMode(ena[i],OUTPUT);
    pinMode(pah[i],OUTPUT);
    Motor[i].setLenth(motor_max);
  }  //モーターのピンと行列式に使う定数の設定
}




void motor_attack::moveMotor_L(angle ang,int val,double ac_val,LINE line){  //モーター制御する関数
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(ang.radians);  //進みたいベクトルのx成分
  double mval_y = sin(ang.radians);  //進みたいベクトルのy成分
  
  max_val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] *(mval_x + line.Lvec_X * line_val)  + mCos[i] *(mval_y + line.Lvec_Y * line_val);

    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //移動平均求めるゾーンだよ
    Mval[i] /= g;  //モーターの値を制御(常に一番大きい値が1になるようにする)

    Mval[i] = Motor[i].demandAve(Mval[i]);

    if(abs(Mval[i]) > h){  //絶対値が一番高い値だったら
      h = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //モーターの値を計算するところだよ
    
    if(i == 0 || i == 3){
      Mval[i] = Mval[i] / h * max_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    }
    else{
      Mval[i] = Mval[i] / h * max_val + ac_val * 1.3;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    }

    if(NoneM_flag == 0){
      Moutput(i,Mval[i]);
    }
  }
  if(NoneM_flag == 1){
    OLED_moving();
  }
}


void motor_attack::moveMotor_0(angle ang,int val,double ac_val,int flag){
  double g = 0;                //モーターの最終的に出る最終的な値の比の基準になる値
  double h = 0;
  double Mval[4] = {0,0,0,0};  //モーターの値×4
  double max_val = val;        //モーターの値の上限値
  double mval_x = cos(ang.radians);  //進みたいベクトルのx成分
  double mval_y = sin(ang.radians);  //進みたいベクトルのy成分
  
  max_val -= ac_val;  //姿勢制御とその他のモーターの値を別に考えるために姿勢制御の値を引いておく
  
  for(int i = 0; i < 4; i++){
    Mval[i] = -mSin[i] * mval_x + mCos[i] * mval_y; //モーターの回転速度を計算(行列式で管理)
    
    if(abs(Mval[i]) > g){  //絶対値が一番高い値だったら
      g = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //移動平均求めるゾーンだよ
    Mval[i] /= g;  //モーターの値を制御(常に一番大きい値が1になるようにする)

    Mval[i] = Motor[i].demandAve(Mval[i]);

    if(abs(Mval[i]) > h){  //絶対値が一番高い値だったら
      h = abs(Mval[i]);    //一番大きい値を代入
    }
  }

  for(int i = 0; i < 4; i++){  //モーターの値を計算するところだよ
    if(flag == 0){
      Mval[i] = Mval[i] / h * max_val + ac_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
    }
    else if(flag == 1){
      if(i == 0 || i == 3){
        Mval[i] = Mval[i] / h * max_val;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
      }
      else{
        Mval[i] = Mval[i] / h * max_val + ac_val * 1.7;  //モーターの値を計算(進みたいベクトルの値と姿勢制御の値を合わせる)
      }
    }

    if(NoneM_flag == 0){
      Moutput(i,Mval[i]);
    }
  }
  if(NoneM_flag == 1){
    OLED_moving();
  }
}


void motor_attack::motor_ac(float ac_val){
  ac_val *= 1.7;
  for(int i = 0; i < 4; i++){
    if(i == 1 || i == 2){
      if(NoneM_flag == 0){
        Moutput(i,ac_val);
      }
    }
    else{
      Moutput(i,0);
    }
  }
  if(NoneM_flag == 1){
    OLED_moving();
  }
}


void motor_attack::motor_0(){  //モーターの値を0にする関数
  for(int i = 0; i < 4; i++){
    digitalWrite(pah[i],LOW);
    analogWrite(ena[i],0);
    Motor[i].reset();
  }
  OLED_moving();
  if(NoneM_flag == 1){
    OLED_moving();
  }
}



float motor_attack::Moutput(int i,float Mval){
  if(i == 1 || i == 3){
    if(0 < Mval){            //モーターの回転方向が正の時
      digitalWrite(pah[i] , LOW);    //モーターの回転方向を正にする
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , HIGH);     //モーターの回転方向を負にする
    }
  }
  else{
    if(0 < Mval){            //モーターの回転方向が正の時
      digitalWrite(pah[i] , HIGH);    //モーターの回転方向を正にする
    }
    else{  //モーターの回転方向が負の時
      digitalWrite(pah[i] , LOW);     //モーターの回転方向を負にする
    }
  }
  analogWrite(ena[i] , abs(Mval)); //モーターの回転速度を設定
  return Mval;
}