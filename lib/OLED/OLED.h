#include<timer.h>
#include<Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include<ball.h>
#include<line.h>
#include<ac.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3C for 128x64, 0x3D for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

class OLED{
    public:
        OLED();
        void setup();
        void displayData(AC,LINE,Ball);
    private:
        int flash_OLED = 0;  //ディスプレイの中で白黒点滅させたいときにつかう
        int OLED_select = 1;  //スイッチが押されたときにどこを選択しているかを示す変数(この数字によって選択画面の表示が変化する)
        int Button_select = 0;  //スイッチが押されたときにどこを選択しているかを示す変数(この数字によってexitかnextかが決まる)
        int aa = 0;  //タクトスイッチのルーレット状態防止用変数
        int A = 0;  //ステートのための変数
        int B = 999;  //ステート初期化のための変数



        //エンコーダの設定
        long oldPosition  = -999;  //エンコーダのオールドポジの初期化
        long new_encVal = 0;  //エンコーダーの現在値を示す変数
        long old_encVal = 0;  //エンコーダーの過去値を示す変数

        unsigned int address = 0x00;  //EEPROMのアドレス

        const int Tact_Switch = 15;  //タクトスイッチのピン番号
        const int Encoder_A = 17;  //エンコーダーのピン番号
        const int Encoder_B = 16;  //エンコーダーのピン番号

        Encoder myEnc(17, 16);  //エンコーダのピン番号

        timer timer_OLED; //タイマーの宣言(OLED用)
        Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
};