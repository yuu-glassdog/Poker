//====================================================================
//  工学部「情報環境実験２」(富永)  C言語プログラミング  例題
//  ポーカーゲームの戦略
//--------------------------------------------------------------------
//  Poker  usr/s13t200_03/  PokerOpe.c
//  Linux CentOS 6.7  GCC 4.4.7
//--------------------------------------------------------------------
//  最所研究室  13T232 近藤裕基
//  2016.01.12
//====================================================================


//====================================================================
//  仕様
//====================================================================

/*--------------------------------------------------------------------

手札、場札、チェンジ数、テイク数、捨札を引数とし、捨札を決める。
返却値は、捨札の位置である。-1のときは、交換しないで、手札を確定させる。
関数 strategy() は、戦略のインタフェースであり、この関数内で、
実際の戦略となる関数を呼び出す。手札と捨札は、不正防止のため、
変更不可なので、局所配列にコピーして、整列などの処理を行う。
複数の戦略を比較したり、パラメタを変化させて、より強い戦略を目指す。

---------------------------------------------------------------------*/


//====================================================================
//  前処理
//====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Poker.h"

int myhn[5];
int num[13] = {0};  // 数位ごとの個数を格納する配列(A23456789TJQK)
int sut[4] = {0};   // 種類ごとの個数を格納する配列(SHDC)

//--------------------------------------------------------------------
//  関数宣言
//--------------------------------------------------------------------

void copy_hd(const int hd[]);
void check_hd(const int hd[]);
void reset_array();
int chance_flash(const int hd[]);

//====================================================================
//  戦略
//====================================================================

/*--------------------------------------------------------------------
//  ユーザ指定
//--------------------------------------------------------------------

最初の手札のまま交換しない。

hd : 手札配列
fd : 場札配列(テイク内の捨札)
cg : チェンジ数
tk : テイク数
ud : 捨札配列(過去のテイクも含めた全ての捨札)
us : 捨札数

--------------------------------------------------------------------*/

int strategy(const int hd[], const int fd[], int cg, int tk, const int ud[], int us) {
    
    int chp = 0;    // 交換する位置
    int k;  // 反復変数
    int t;  // 一時変数

    copy_hd(hd);    // 手札の複製
    check_hd(hd);   // 手札の確認
    
    // フラッシュ
    if ( ( chp = chance_flash(hd) ) != -1 && cg < 5) {
        return chp;
    }

    // 1テイク目は捨てる
    if ( tk < 2 ) { reset_array(); return -1; }
    if ( poker_point(myhn) > P2 ) { reset_array(); return -1; }

    return 0;
}


//====================================================================
//  補助関数
//====================================================================

// 手札の複製
void copy_hd(const int hd[]) {
    
    int k;    // 反復変数

    for ( k = 0; k < 5; k++ ){
        myhn[k] = hd[k];
    }
}

// 手札の確認
void check_hd(const int hd[]) {
    
    int k;  // 反復変数
    int t;  // 一時変数
    
    // 数位および種類ごとの計数
    for( k = 0; k < HNUM; k++ ) { t = hd[k] % 13; num[t]++; }   // 数位
    for( k = 0; k < HNUM; k++ ) { t = hd[k] / 13; sut[t]++; }   // 種類
}

// 各配列のリセット
void reset_array(void) {
    
    int k;
    
    for ( k = 0; k < HNUM; k++ ) { myhn[k] = 0; }
    for ( k = 0; k < 13; k++ ) { num[k] = 0; }
    for ( k = 0; k < 4; k++ ) { sut[k] = 0; }
}

// あと1枚でフラッシュか確認
int chance_flash(const int hd[]) {
    
    int chp;        // 交換すべき位置
    int high, low;  // 各種類の境界値
    int k;
    int j;

    for ( k = 0; k < 4; k++ ) {
        // 4種どれかが4枚ある場合
        if ( sut[k] == 4 ) {
            // 境界値の確認
            switch(k){
                case 0: low = 0; high = 12; break;  // スペード
                case 1: low = 13; high = 25; break; // ハート
                case 2: low = 26; high = 38; break; // ダイヤ
                case 3: low = 39; high = 51; break; // クラブ
            }
            
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                if ( hd[j] < low || high < hd[j] ) { return j; }
            }
            
        }
    }
    
    // フラッシュが望めない場合
    return -1;
}