#include <M5StickCPlus.h>
#include "time.h"

#define SAMPLE_PERIOD 20    // サンプリング間隔(ミリ秒)
#define SAMPLE_SIZE 200     // 20ms x 200 = 4秒

unsigned long tmStart=0,tmEnd=0,tm=0; 
  // tmStart:最大加速度を記録した時刻
  // tmEnd : 最大加速度の1/3を下回った時刻
  // tm:減衰所要時間
int tmCount=0;   // tmCount 0:未計測　1:計測中　2:計測終了
int wvCount = 0,wvn = 0; // wvCount 減衰波の数　-1:計測前　0:最大値計測時　1以降:n番目の波
void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.IMU.Init();  // MPU6886を初期設定する
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE,BLACK);
    Serial.begin(115200);             // シリアル通信の準備をする
    while (!Serial);                  // 準備が終わるのを待つ
    Serial.println("Program Start");
    
}

float ax, ay, az[SAMPLE_SIZE],wv[SAMPLE_SIZE];  // 加速度データを読み出す変数,wv:振幅記録用
float maxaz = 0.0;
float aveaz = 0.0;

#define X0 5  // 横軸の描画開始座標
#define MINZ 600  // 縦軸の最小値 600mG
#define MAXZ 1400  // 縦軸の最大値 1400mG

void loop() {
    M5.update();
    while (!M5.BtnA.wasPressed()){
         M5.update();
         tmCount = 0;
         tmStart = 0;
         tmEnd = 0;
         tm = 0;
         maxaz = 0.0;
         wvCount = 0;
    }
    M5.Lcd.fillScreen(BLACK);  // 画面をクリア
    M5.Lcd.setCursor(0, 0);
//    M5.Lcd.printf("P:%.0fmG wvC:%d     ", maxaz,wvCount);
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        M5.IMU.getAccelData(&ax,&ay,&az[i]);  // MPU6886から加速度を取得

        az[i] *= 1000;  // mGに変換

        if (i < 2) continue;

        int y0 = map((int)(az[i - 2]), MINZ, MAXZ, M5.Lcd.height(), 0);
        int y1 = map((int)(az[i - 1]), MINZ, MAXZ, M5.Lcd.height(), 0);
        M5.Lcd.drawLine(i - 1 + X0, y0, i + X0, y1, GREEN);
        M5.Lcd.setCursor(0, 0);
//        M5.Lcd.printf("P:%.0fmG T:%dmS     ", maxaz,tm);
        M5.Lcd.printf("P:%.0fmG wvn:%d     ", maxaz,wvn);
 //       Serial.println(az[i]);
        if (i > 4) {
          if ( (az[i-4]-az[i-2])*(az[i-2]-az[i]) < 0){
            wvCount ++;
            wv[wvCount] = abs(az[i-2]); 
            M5.Lcd.drawRect(i - 1 + X0, y0, 2, 2, RED);    
          }
        }
        if (az[i-2] > maxaz){
          maxaz = az[i-2];
          tmStart = millis();
          tmCount = 1;
          M5.Lcd.drawRect(i - 1 + X0, y0, 2, 2, WHITE);
        }
        if (( ( (maxaz-0.98) * 0.1) > (wv[wvCount]-0.98) ) && (tmCount == 1) ){
          tmEnd = millis();
          wvn = wvCount;
          tmCount = 2;
          tm = tmEnd - tmStart;
        }
        delay(SAMPLE_PERIOD);
        
/*        if ( M5.BtnA.wasPressed() ) {
          tmCount = 0;
          tmStart = 0;
          tmEnd = 0;
          tm = 0;
          maxaz = 0.0;
          wvCount = 0;
//          M5.Lcd.fillScreen(BLACK);  // 画面をクリア
        }
*/
    }
}
