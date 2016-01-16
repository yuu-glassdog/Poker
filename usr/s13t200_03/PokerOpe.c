//====================================================================
//  工学部「情報環境実験２」(富永)  C言語プログラミング  例題
//  ポーカーゲームの戦略
//--------------------------------------------------------------------
//  Poker  usr/s13t200_03/  PokerOpe.c
//  Linux CentOS 6.7  GCC 4.4.7
//--------------------------------------------------------------------
//  最所研究室  13T232 近藤裕基
//  2016.01.17
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
int num[NUM] = {0};     // 各位の手札内枚数を格納する配列(A23456789TJQK)
int sut[SUT] = {0};     // 各種の手札内枚数を格納する配列(SHDC)
int unum[NUM] = {0};    // 各位の捨て札数を格納する配列
int usut[SUT] = {0};    // 各種の捨て札数を格納する配列
int chp[HAND] = {0};    // 交換すべき札の重み付け配列

//--------------------------------------------------------------------
//  関数宣言
//--------------------------------------------------------------------

void check_card(const int hd[], const int ud[], int us);
void reset_array(void);
void chance_flush(const int hd[]);
void chance_4card(const int hd[]);
void chance_fullhause(const int hd[]);
void chance_straight(const int hd[]);
int make_pair(const int hd[]);
int change_card(const int hd[]);

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
    check_card(hd, ud, us);     // 手札の確認
    
    // 得点がフルハウス以上ならOK
    if ( poker_point(myhn) >= P6 ) { return -1; }
    
    // フラッシュの可能性を確認
    chance_flush(hd);
    
    // フルハウスの可能性を確認
    chance_fullhause(hd);
    
    // フォーカードの可能性を確認
    chance_4card(hd);
    
    // ストレートの可能性を確認
    chance_straight(hd);

    // 交換するカードを決定
    return change_card(hd);
}

//====================================================================
//  補助関数
//====================================================================

// 手札の確認
void check_card(const int hd[], const int ud[], int us) {
    
    int k;      // 反復変数
    int t1, t2; // 一時変数
    
    // 数位および種類ごとの計数
    for( k = 0; k < HNUM; k++ ) {
        t1 = hd[k] / NUM;
        t2 = hd[k] % NUM;
        sut[t1]++;          // 手札の種類
        num[t2]++;          // 手札の数位
    }
    for( k = 0; k < us; k++ ) {
        t1 = ud[k] / NUM;
        t2 = ud[k] % NUM;
        usut[t1]++;         // 捨て札の種類
        unum[t2]++;         // 捨て札の数位
    }
}

// 各配列のリセット
void reset_array(void) {
    
    int k;
    
    for ( k = 0; k < HNUM; k++ ) { myhn[k] = 0; chp[k] = 0;}
    for ( k = 0; k < NUM; k++ ) { num[k] = 0; unum[k] = 0; }
    for ( k = 0; k < SUT; k++ ) { sut[k] = 0; usut[k] = 0; }
}

// あと1枚でフラッシュか確認
void chance_flush(const int hd[]) {
    
    int i, j;
    int t;

    for ( i = 0; i < SUT; i++ ) {
        // 4種どれかが3枚か4枚ある場合
        if ( sut[i] == SUT-1 || sut[i] == SUT ){
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                t = hd[j] / NUM;
                if ( t != i ) { chp[j] += 2; }
            }
        }
    }
}

// あと1枚でフォーカードか確認
void chance_4card(const int hd[]) {
    
    int target; // 狙うカードの位
    int i, j;
    int t;

    for ( i = 0; i < NUM; i++ ) {
        // 同位札が3枚ある(スリーカインズ)の場合
        if ( num[i] == 3 ) {
            target = i;
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                t = hd[j] % NUM;
                if ( t != target ) { chp[j] += 4; }
            }
            
        }
    }
}

// あと1枚でフルハウスか確認
void chance_fullhause(const int hd[]) {
    
    int ct = 0;             // ペアの数
    int target[2] = {0};    // ペアがある位
    int three = 0;          // スリーカインズのフラッグ
    int i, j;
    int t;

    for ( i = 0; i < NUM; i++ ) {
        // ペアを数える
        if ( num[i] == 2 ) { target[ct] = i; ct++; }
        // スリーカインズがある場合
        if ( num[i] == 3 ) { target[0] = i; target[1] = i; three = 1; }
        // ペアが2つある(ツーペア)の場合
        if ( ct == 2 || three ) {           
            // 交換すべき位置を探索して返却
            for ( j = 0; j < HNUM; j++ ) {
                t = hd[j] % NUM;
                if ( t != target[0] && t != target[1] ) {
                    chp[j] += 1;
                }
            }            
        }
    }
}

// あと1枚でストレートか確認
void chance_straight(const int hd[]) {
    
    int ct[NUM-3] = {0};    // 5数による組中で1枚の位
    int target;             // 抜けている数
    int i, j;
    int t;

    for ( i = 0; i < NUM-3; i++ ) {
        // k ～ k+4 までで1枚の位がいくつあるか数える
        for ( j = i; j < i+5; j++ ) {
            // 10～A のときのA
            if ( j == 13 && num[0] == 1 ) { ct[i]++; } 
            // それ以外
            else if ( num[j] == 1 ) { ct[i]++; }
        }
        // 4枚の順位札が揃っている場合
        if ( ct[i] == 4 ) {
            // 抜けている数の確認
            for ( j = i; j < i+5; j++ ) {
                // 10～AのときのA
                if ( j == 13 && num[0] == 0 ) { target = 0; break; }
                // それ以外
                else if ( num[j] == 0 ) { target = j; break; }
            }
            // 手札から要らない札を識別して重み付け
            for ( j = 0; j < HNUM; j++ ) {
                t = hd[j] % NUM;
                //if ( t == target && unum[t] != 4 ) { chp[j] += 3; }
                if ( t == target ) { chp[j] += 3; }
            }
        }
    }
}

// 先読みして役を作る試み
int make_pair(const int hd[]){
    
    int save[HNUM] = {0};   // 残すべき札の重み付け配列
    int point;              // 交換する札の位置
    int max = 0;
    int max2 = 0;
    int min = NUM;
    int t1, t2;
    int k;

    // 残すべき札の重み付け
    for( k = 0; k < HNUM; k++ ) {
        t1 = hd[k] / NUM;
        t2 = hd[k] % NUM;
        // 同種札がある
        if ( sut[t1] >= 2 ) { save[k] += 6; }
        // 同位札がある
        if ( num[t2] >= 2 ) { save[k] += 7; }
        // 10JQKAである
        if ( t2 >= 9 || t2 == 0 ) { save[k] += 1; }
        // 捨て札数が最も多い位および種の札を確認
        if ( usut[t1] > max ) { max = k; }
        if ( unum[t2] > max2 ) { max2 = k; }
    }
    // 捨て札数が最も多い位および種の札から重みを減らす
    save[max] -= 1;
    save[max2] -= 2;

    // 交換する札の決定
    for ( k = 0; k < HNUM; k++ ) {
        if ( save[k] < min ) { min = save[k]; point = k; }
    }
    return point;
}

// 交換する札の決定
int change_card(const int hd[]) {
    
    int max = 0;
    int point = 0;
    int k;
    
    // 狙う役のチャンスがある場合
    for ( k = 0; k < HNUM; k++ ) {
        if ( chp[k] > max ) { max = chp[k]; point = k; }
    }
    // 狙いが外れている場合
    if ( max == 0 ) { point = make_pair(hd); } 

    return point;
}
