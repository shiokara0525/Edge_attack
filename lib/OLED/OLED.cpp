#include<OLED.h>

OLED::OLED(){
    EEPROM.get(address,LINE_Level);//EEPROMから読み出し
    address += sizeof(LINE_Level);  //アドレスを次の変数のアドレスにする
    EEPROM.get(address,RA_size);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
    address += sizeof(RA_size);  //アドレスを次の変数のアドレスにする
    EEPROM.get(address,val_max);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
    }

    //OLEDの初期化
    display.display();
    display.clearDisplay();

    timer_OLED.reset(); //タイマーのリセット(OLED用)
}


void OLED::displayData(AC ac,LINE line,Ball ball){
  if(timer_OLED.read_ms() > 500) //0.5秒ごとに実行(OLEDにかかれてある文字を点滅させるときにこの周期で点滅させる)
  {
    if(flash_OLED == 0){
      flash_OLED = 1;
    }
    else{
      flash_OLED = 0;
    }
    timer_OLED.reset(); //タイマーのリセット(OLED用)
  }


  if(A == 0)  //メインメニュー
  {
    if(A != B)  //ステートが変わったときのみ実行(初期化)
    {
      OLED_select = 1;  //選択画面をデフォルトにする
      B = A;
    }

    //OLEDの初期化
    display.display();
    display.clearDisplay();

    //選択画面だということをしらせる言葉を表示
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Hi! bro!");
    display.setCursor(0,10);
    display.println("What's up?");

    //文字と選択画面の境目の横線を表示
    display.drawLine(0, 21, 128, 21, WHITE);

    //選択画面の表示
    if(OLED_select == 1)  //STARTを選択しているとき
    {
      //START値を調整
      display.setTextSize(2);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(0,35);
      display.println("START");

      //選択画面で矢印マークを中央に表示
      display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(90,35);
      display.println("Set");
      display.setCursor(88,45);
      display.println("Line");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A = 10;  //その選択されているステートにレッツゴー
          aa = 0;
        }
      }
    }
    else if(OLED_select == 2)  //Set Lineを選択しているとき
    {
      //Line値を調整
      display.setTextSize(2);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(12,27);
      display.println("Set");
      display.setCursor(6,44);
      display.println("Line");

      //選択画面で矢印マークを中央に表示
      display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(85,35);
      display.println("Check");
      display.setCursor(88,45);
      display.println("Line");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A = 20;  //その選択されているステートにレッツゴー
          aa = 0;
        }
      }
    }
    else if(OLED_select == 3)  //Check Lineを選択しているとき
    {
      //Check Lineの文字設定
      display.setTextSize(2);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(0,27);
      display.println("Check");
      display.setCursor(6,44);
      display.println("Line");

      //選択画面で矢印マークを中央に表示
      display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(90,35);
      display.println("Set");
      display.setCursor(94,45);
      display.println("RA");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A = 30;  //その選択されているステートにレッツゴー
          aa = 0;
        }
      }
    }
    else if(OLED_select == 4)  //Set RA（回り込みの大きさ）を選択しているとき
    {
      //回り込みの大きさを調整
      display.setTextSize(2);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(12,27);
      display.println("Set");
      display.setCursor(18,44);
      display.println("RA");

      //選択画面で矢印マークを中央に表示
      display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(85,35);
      display.println("Check");
      display.setCursor(88,45);
      display.println("Ball");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A = 40;  //その選択されているステートにレッツゴー
          aa = 0;
        }
      }
    }
    else if(OLED_select == 5)  //Check Ballを選択しているとき
    {
      //Check Ballの文字設定
      display.setTextSize(2);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(0,27);
      display.println("Check");
      display.setCursor(6,44);
      display.println("Ball");

      //選択画面で矢印マークを中央に表示
      display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(90,35);
      display.println("Set");
      display.setCursor(88,44);
      display.println("Motar");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A = 50;  //その選択されているステートにレッツゴー
          aa = 0;
        }
      }
    }
    else if(OLED_select == 6)  //Set Motarを選択しているとき
    {
      //Motar値を調整
      display.setTextSize(2);
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
      display.setCursor(12,27);
      display.println("Set");
      display.setCursor(0,44);
      display.println("Motar");

      //選択画面で矢印マークを中央に表示
      display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(85,40);
      display.println("START");

      //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
      if(aa == 0){
        if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
          aa = 1;
        }
      }else{
        if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
          A = 60;  //その選択されているステートにレッツゴー
          aa = 0;
        }
      }
    }
  }
  else if(A == 10)  //START
  { //機体の中心となるコート上での0°の位置を決めるところ
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };

    //OLEDの初期化
    display.display();
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(30,0);
    display.println("Please");
    display.setCursor(0,20);
    display.println("CAL");
    display.setCursor(40,20);
    display.println("&");
    display.setCursor(56,20);
    display.println("SetDir");

    display.setTextSize(1);
    display.setCursor(38,40);
    display.println("of BNO055");

    display.setTextColor(WHITE);
    if(Button_select == 1)  //exitが選択されていたら
    {
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
    }
    display.setCursor(0,56);
    display.println("Exit");

    display.setTextColor(WHITE);
    if(Button_select == 0)  //nextが選択されていたら（デフォルトはこれ）
    {
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
    }
    display.setCursor(104,56);
    display.println("Next");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        if(Button_select == 0)  //nextが選択されていたら
        {
          /*******************************************************************************ここで角度を決定*/
          A = 15;  //スタート画面に行く
        }
        else if(Button_select == 1)  //exitが選択されていたら
        {
          A = 0;  //メニュー画面に戻る
        }
        aa = 0;
      }
    }
  }
  else if(A == 15)  //ボタン押したらロボット動作開始
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };

    //OLEDの初期化
    display.display();
    display.clearDisplay();

    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(22,0);
    display.println("START");

    display.setTextSize(1);
    display.setCursor(38,35);
    display.println("ac.dir :");
    display.setTextSize(2);
    display.setCursor(80,30);
    display.println(ac.dir);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    if(Button_select == 1)  //exitが選択されていたら
    {
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
    }
    display.setCursor(0,56);
    display.println("Exit");

    display.setTextColor(WHITE);
    if(Button_select == 0)  //nextが選択されていたら（デフォルトはこれ）
    {
      if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
        display.setTextColor(BLACK, WHITE);
      }
      else{
        display.setTextColor(WHITE);
      }
    }
    display.setCursor(56,55);
    display.println("SetDir Again");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        if(Button_select == 0)  //SetDir Againが選択されていたら
        {
          ac.dir = 300;  //ここで現在の角度を0°（基準）とする
        }
        else if(Button_select == 1)  //exitが選択されていたら
        {
          A = 0;  //メニュー画面に戻る
        }
        aa = 0;
      }
    }
  }
  else if(A == 20)  //Set Line
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };

    display.display();
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(16,0);
    display.println("Set Line");

    display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
    display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

    //数字を中央揃えにするためのコード
    display.setTextSize(3);
    display.setTextColor(WHITE);
    if(LINE_Level >= 1000){      //4桁の場合
      display.setCursor(28,22);
    }else if(LINE_Level >= 100){ //3桁の場合
      display.setCursor(40,22);
    }else if(LINE_Level >= 10){  //2桁の場合
      display.setCursor(48,22);
    }else{                       //1桁の場合
      display.setCursor(56,22);
    }
    display.println(LINE_Level);  //ラインの閾値を表示

    display.setTextSize(1);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(44,56);
    display.println("Confirm");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    //タクトスイッチが押されたら、メニューに戻る
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        address = 0x00;  //EEPROMのアドレスを0x00にする
        // LINE_Level = 700;  //初めにデータをセットしておかなければならない
        EEPROM.put(address, LINE_Level);  //EEPROMにラインの閾値を保存
        A = 0;  //メニュー画面へ戻る
        aa = 0;
      }
    }
  }
  else if(A == 30)  //Check Line
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };
    
    display.display();
    display.clearDisplay();

    //ラインの位置状況マップを表示する
    display.drawCircle(32, 32, 20, WHITE);  //○ 20

    //ラインの位置状況を表示する
    /*ラインの線の座標をOLEDでの座標に変換(-1~1の値を2~62の値に変換)*/
    Ax_OLED = map(Ax, -1,1, 2,62);
    Ay_OLED = map(Ay, -1,1, 2,62);
    Bx_OLED = map(Bx, -1,1, 2,62);
    By_OLED = map(By, -1,1, 2,62);
    display.drawLine(Ax_OLED, Ay_OLED, Bx_OLED, By_OLED, WHITE); //ラインの線を表示

    //"Line"と表示する
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(68,0);
    display.println("Line");

    //ここから下のコードのテキストをsize1にする
    display.setTextSize(1);
    display.setTextColor(WHITE);

    //ラインの角度を表示する
    display.setCursor(68,25);
    display.println("ac.dir:");
    if(LINE_on == 1){  //ラインがロボットの下にある
      display.setCursor(96,25);
      display.println(Lvec_Dir);
    }
    else{  //ラインがロボットの下にない
      display.fillRect(96, 25, 34, 10, WHITE);
    }

    //ラインの距離を表示する
    display.setCursor(68,39);
    display.println("far:");
    if(LINE_on == 1){  //ラインがロボットの下にある
      display.setCursor(96,39);
      display.println(Lvec_long);
    }
    else{  //ラインがロボットの下にない
      display.fillRect(96, 39, 34, 10, WHITE);
    }

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    //タクトスイッチが押されたら、メニューに戻る
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        A = 0;  //メニュー画面へ戻る
        aa = 0;
      }
    }

    // //白線の平均値を表示する
    // display.setCursor(68,44);
    // display.println("Whi:");
    // display.setCursor(96,44);
    // display.println(Lwhite);

    // //緑コートの平均値を表示する
    // display.setCursor(68,56);
    // display.println("Gre:");
    // display.setCursor(96,56);
    // display.println(Lgreen);
  }
  else if(A == 40)  //Set RA
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };

    display.display();
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(16,0);
    display.println("Set RA");

    display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
    display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

    //数字を中央揃えにするためのコード
    display.setTextSize(3);
    display.setTextColor(WHITE);
    if(RA_size >= 1000){      //4桁の場合
      display.setCursor(28,22);
    }else if(RA_size >= 100){ //3桁の場合
      display.setCursor(40,22);
    }else if(RA_size >= 10){  //2桁の場合
      display.setCursor(48,22);
    }else{                       //1桁の場合
      display.setCursor(56,22);
    }
    display.println(RA_size);  //ボールの閾値を表示

    display.setTextSize(1);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(44,56);
    display.println("Confirm");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    //タクトスイッチが押されたら、メニューに戻る
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        address = 0x00;  //EEPROMのアドレスを0x00にする（リセット）
        address += sizeof(LINE_Level);  //アドレスを次の変数のアドレスにする
        RA_size = 80;  //初めにデータをセットしておかなければならない
        EEPROM.put(address, RA_size);  //EEPROMにボールの閾値を保存
        A = 0;  //メニュー画面へ戻る
        aa = 0;
      }
    }
  }
  else if(A == 50)  //Check Ball
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };

    display.display();
    display.clearDisplay();

    //ボールの位置状況マップを表示する
    display.drawCircle(32, 32, 30, WHITE);  //○ 30
    display.drawCircle(32, 32, 20, WHITE);  //○ 20
    display.drawCircle(32, 32, 10, WHITE);  //○ 10
    display.drawLine(2, 32, 62, 32, WHITE); //-
    display.drawLine(32, 2, 32, 62, WHITE); //|

    //"Ball"と表示する
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(68,0);
    display.println("Ball");

    //ここから下のコードのテキストをsize1にする
    display.setTextSize(1);
    display.setTextColor(WHITE);

    //ボールの角度を表示する
    display.setCursor(68,24);
    display.println("ac.dir:");
    if(ball_flag == 1){  //ボールがあれば値を表示
      display.setCursor(96,24);
      display.println(ball.ang);
    }
    else{  //ボールがなければ白い四角形を表示
      display.fillRect(96, 24, 34, 10, WHITE);
    }

    //ボールの距離を表示する
    display.setCursor(68,38);
    display.println("far:");
    if(ball_flag == 1){  //ボールがあれば値を表示
      display.setCursor(96,38);
      display.println(ball.far);
    }
    else{  //ボールがなければ白い四角形を表示
      display.fillRect(96, 38, 34, 10, WHITE);
    }

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    //タクトスイッチが押されたら、メニューに戻る
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        A = 0;  //メニュー画面へ戻る
        aa = 0;
      }
    }
  }
  else if(A == 60)  //Set Motar
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };

    display.display();
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(14,0);
    display.println("Set Motar");

    display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
    display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

    //数字を中央揃えにするためのコード
    display.setTextSize(3);
    display.setTextColor(WHITE);
    if(val_max >= 1000){      //4桁の場合
      display.setCursor(28,22);
    }else if(val_max >= 100){ //3桁の場合
      display.setCursor(40,22);
    }else if(val_max >= 10){  //2桁の場合
      display.setCursor(48,22);
    }else{                       //1桁の場合
      display.setCursor(56,22);
    }
    display.println(val_max);  //ラインの閾値を表示

    display.setTextSize(1);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(44,56);
    display.println("Confirm");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    //タクトスイッチが押されたら、メニューに戻る
    if(aa == 0){
      if(digitalRead(Tact_Switch) == LOW){  //タクトスイッチが押されたら
        aa = 1;
      }
    }else{
      if(digitalRead(Tact_Switch) == HIGH){  //タクトスイッチが手から離れたら
        address = 0x00;  //EEPROMのアドレスを0x00にする（リセット）
        address = sizeof(LINE_Level) + sizeof(RA_size);  //アドレスを次の変数のアドレスにする
        // val_max = 100;  //初めにデータをセットしておかなければならない
        EEPROM.put(address, val_max);  //EEPROMにボールの閾値を保存
        A = 0;  //メニュー画面へ戻る
        aa = 0;
      }
    }
  }

  //ロータリーエンコーダーの値を取得し制御する
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    if(newPosition % 4 == 0)  //4の倍数のときのみ実行
    {
      new_encVal = newPosition / 4;  //Aにステートを代入
      if(A == 0)  //選択画面にいるときはOLED_selectを変更する
      {
        if(new_encVal > old_encVal)  //回転方向を判定
        {
          OLED_select++;  //次の画面へ
          if(OLED_select > 6)  //選択画面の数以上になったら1に戻す
          {
            OLED_select = 1;
          }
        }
      }
      else if(A == 10 || A == 15)  //スタート画面にいるときはButton_selectを変更する
      {
        if(new_encVal > old_encVal)  //回転方向を判定
        {
          Button_select = 0;  //next
        }
        else if(new_encVal < old_encVal)
        {
          Button_select = 1;  //exit
        }
      }
      else if(A == 20)  //ラインの閾値を変更する
      {
        if(new_encVal > old_encVal)  //回転方向を判定
        {
          if(LINE_Level < 1023)
          {
            LINE_Level++;
          }
        }
        else if(new_encVal < old_encVal)
        {
          if(LINE_Level > 0)
          {
            LINE_Level--;
          }
        }
      }
      else if(A == 40)  //ボールの閾値を変更する
      {
        if(new_encVal > old_encVal)  //回転方向を判定
        {
          if(RA_size < 1023)
          {
            RA_size++;
          }
        }
        else if(new_encVal < old_encVal)
        {
          if(RA_size > 0)
          {
            RA_size--;
          }
        }
      }
      else if(A == 60)  //モーターの出力を変更する
      {
        if(new_encVal > old_encVal)  //回転方向を判定
        {
          if(val_max < 1023)
          {
            val_max++;
          }
        }
        else if(new_encVal < old_encVal)
        {
          if(val_max > 0)
          {
            val_max--;
          }
        }
      }
      old_encVal = new_encVal;
    }
  }
}
}