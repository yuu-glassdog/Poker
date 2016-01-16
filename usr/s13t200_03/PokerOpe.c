//====================================================================
//  工学部「情報環境実験２」(富永)  C言語プログラミング  例題
//  ポーカーゲームの戦略
//--------------------------------------------------------------------
//  Poker  usr/s13t200_03/  PokerOpe.c
//  Linux CentOS 6.7  GCC 4.4.7
//--------------------------------------------------------------------
//  最所研究室  13T232 近藤裕基
//  2016.01.15
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

#define HAND 5
#define NUM 13
#define SUT 4

int myhn[HAND];
int num[NUM] = {0};     // 数位ごとの個数を格納する配列(A23456789TJQK)
int sut[SUT] = {0};     // 種類ごとの個数を格納する配列(SHDC)
int chp[HAND] = {0};    // 交換すべき札の重み付け配列

//--------------------------------------------------------------------
//  関数宣言
//--------------------------------------------------------------------

void copy_hd(const int hd[]);
void check_hd(const int hd[]);
void reset_array();
void chance_flash(const int hd[]);
void chance_4card(const int hd[]);
void chance_fullhause(const int hd[]);
void chance_straight(const int hd[]);
int make_pair(const int hd[], const int ud[], int us);
int change_card(const int hd[], const int ud[], int us);

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
    
    int k;  // 反復変数
    int t;  // 一時変数
    
    reset_array();              // 各配列の初期化
    arr_copy(myhn, hd, HNUM);   // 手札の複製
    check_hd(hd);               // 手札の確認
    
    // 得点がフルハウス以上ならOK
    if ( poker_point(myhn) >= P6 ) { return -1; }
    
    // フラッシュの可能性を確認
    chance_flash(hd);
    
    // フルハウスの可能性を確認
    chance_fullhause(hd);
    
    // フォーカードの可能性を確認
    chance_4card(hd);
    
    // ストレートの可能性を確認
    chance_straight(hd);

    // 交換するカードを決定
    return change_card(hd, ud, us);
}

//====================================================================
//  補助関数
//====================================================================

// 手札の確認
void check_hd(const int hd[]) {
    
    int k;  // 反復変数
    int t;  // 一時変数
    
    // 数位および種類ごとの計数
    for( k = 0; k < HNUM; k++ ) { t = hd[k] % NUM; num[t]++; }   // 数位
    for( k = 0; k < HNUM; k++ ) { t = hd[k] / NUM; sut[t]++; }   // 種類
}

// 各配列のリセット
void reset_array(void) {
    
    int k;
    
    for ( k = 0; k < HNUM; k++ ) { myhn[k] = 0; chp[k] = 0;}
    for ( k = 0; k < NUM; k++ ) { num[k] = 0; }
    for ( k = 0; k < SUT; k++ ) { sut[k] = 0; }
}

// あと1枚でフラッシュか確認
void chance_flash(const int hd[]) {
    
    int k;
    int j;

    for ( k = 0; k < SUT; k++ ) {
        // 4種どれかが3枚か4枚ある場合
        if ( sut[k] == SUT-1 || sut[k] == SUT ){
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                if ( hd[j] / NUM != k ) { chp[j] += 5; }
            }
        }
    }
}

// あと1枚でフォーカードか確認
void chance_4card(const int hd[]) {
    
    int target; // 狙うカードの位
    int k;
    int j;

    for ( k = 0; k < NUM; k++ ) {
        // 同位札が3枚ある(スリーカインズ)の場合
        if ( num[k] == 3 ) {
            target = k + 1;
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                if ( hd[j] % NUM != target ) { chp[j] += 7; }
            }
            
        }
    }
}

// あと1枚でフルハウスか確認
void chance_fullhause(const int hd[]) {
    
    int ct = 0;             // ペアの数
    int target[2] = {0};    // ペアがある位
    int k;
    int j;

    for ( k = 0; k < NUM; k++ ) {
        // ペアを数える
        if ( num[k] == 2 ) { target[ct] = k; ct++; }
        // ペアが2つある(ツーペア)の場合
        if ( ct == 2 ) {           
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                if ( hd[j] % NUM != target[0] && hd[j] % NUM != target[1]) { chp[j] += 4; }
            }            
        }
    }
}

// あと1枚でストレートか確認
void chance_straight(const int hd[]) {
    
    int ct[NUM-3] = {0};    // 5数による組中で1枚の位
    int target;             // 抜けている数
    int k;
    int j;

    for ( k = 0; k < NUM-3; k++ ) {
        // k ～ k+4 までで1枚の位がいくつあるか数える
        for ( j = k; j < k+5; j++ ) {
            // 10～A のときのA
            if ( j == 13 && num[0] == 1 ) { ct[k]++; } 
            // それ以外
            else if ( num[j] == 1 ) { ct[k]++; }
        }
        // 4枚の順位札が揃っている場合
        if ( ct[k] == 4 ) {
            // 抜けている数の確認
            for ( j = k; j < k+5; j++ ) {
                // 10～AのときのA
                if ( j == 13 && num[0] == 0 ) { target = 0; break; }
                // それ以外
                else if ( num[j] == 0 ) { target = j; break; }
            }
            // 手札から要らない札を識別
            for ( j = 0; j < HNUM; j++ ) {
                if ( hd[j] % NUM == target ) { chp[j] += 6; }
            }
        }
    }
}

// 先読みしてペアを作る試み
int make_pair(const int hd[], const int ud[], int us){
    
    int unum[NUM] = {0};    // 各位の捨て札数
    int save[HNUM] = {0};   // 残すべき札の重み付け配列
    int point;              // 交換する札の位置
    int max = 0;
    int min = NUM;
    int k;

    // 残すべき札の重み付け
    for( k = 0; k < HNUM; k++ ) {
        // 同種札がある
        if ( sut[hd[k] / NUM] >= 2 ) { save[k] += 3; }
        // 同位札がある
        if ( num[hd[k] % NUM] >= 2 ) { save[k] += 5; }
        // 10JQKAである
        if ( hd[k] % NUM >= 9 || hd[k] % NUM == 0 ) { save[k] += 1; }
    }

    // 各位の捨て札数を確認
    for( k = 0; k < us; k++ ) { unum[ud[k] % NUM]++; }
    // 捨て札数が最も多い位の札から重みを減らす
    for( k = 0; k < HNUM; k++ ) {
        if ( unum[hd[k] % NUM] > max ) { max = k; }
    }
    save[max] -= 1;
    
    // 交換する札の決定
    for ( k = 0; k < HNUM; k++ ) {
        if ( save[k] < min ) { min = save[k]; point = k; }
    }
    return point;
}

// 交換する札の決定
int change_card(const int hd[], const int ud[], int us) {
    
    int max = 0;
    int point = 0;
    int k;
    
    // 狙う役のチャンスがある場合
    for ( k = 0; k < HNUM; k++ ) {
        if ( chp[k] > max ) { max = chp[k]; point = k; }
    }
    // 狙いが外れている場合
    if ( max == 0 ) { point = make_pair(hd, ud, us); } 

    return point;
}
