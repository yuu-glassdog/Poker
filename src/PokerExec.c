//====================================================================
//  工学部「情報環境実験２」(富永)  C言語プログラミング  例題
//  ポーカーゲームの戦略  ゲーム本体
//--------------------------------------------------------------------
//  Poker  src/ PokerExec.c
//  Linux CentOS 6.7  GCC 4.7.7
//--------------------------------------------------------------------
//  富永研究室  tominaga 富永浩之
//  2015.11.10
//====================================================================


//====================================================================
//  仕様
//====================================================================

/*--------------------------------------------------------------------

減一型(手札から1枚を捨ててから、山札から1枚を加える)

---------------------------------------------------------------------*/


//====================================================================
//  事前処理
//====================================================================

//--------------------------------------------------------------------
//  ヘッダファイル
//--------------------------------------------------------------------

#include <stdio.h>     // 標準優出力
#include <stdlib.h>    // 標準ライブラリ
#include <string.h>    // 文字列処理
#include <time.h>      // 時間関数
#include <math.h>      // 数学関数
#include <dirent.h>    // ディレクトリ操作

#include "Poker.h"     // 課題ライブラリのヘッダファイル(相対パス指定)


//====================================================================
//  大域宣言
//====================================================================

//--------------------------------------------------------------------
//  関数宣言
//--------------------------------------------------------------------

// ヘッダファイルPoker.hをインクルード

//--------------------------------------------------------------------
//  大域変数
//--------------------------------------------------------------------

char Name[GAME_NAME_LEN];         // ゲーム名
int Trial;                        // トライ回数
char Deck[FILE_NAME_LEN];         // 山札ファイル
int Disp_Mode;                    // 表示モード
char Dir[DIR_NAME_LEN];           // ログディレクトリへのパス名
FILE *Game_Log;                   // ゲームログへのファイルポインタ
FILE *Result_Log;                     // 結果ログへのファイルポインタ
int Hand_Value[10] = {P0, P1, P2, P3, P4, P5, P6, P7, P8, P9};
double Take_Weight[10] = {1.0, 1.5, 2.0, 1.5, 1.0};   // 傾斜掛率(2015年)


//====================================================================
//  本体関数
//====================================================================

int main(int argc, char *argv[])
{
  //----  局所宣言
  int point[GNUM][TAKE];    // 各回得点
  FILE *fp;                 // 山札へのファイルポインタ
  double total;             // 総合得点
  double ave;               // 平均得点
  int k;                    // 反復変数

  //----  コマンド引数の確認
  if ( argc < 5 ) {
    puts("ERROR : コマンド引数が不正です。");
    puts("実行パラメタ : ゲーム名 トライ回数 山札ファイル 表示モード");
    exit(EXIT_FAILURE);
  }

  //----  コマンド引数の格納
  strcpy(Name, argv[1]);        // ゲーム名
  Trial = atoi(argv[2]);        // トライ回数
  strcpy(Deck, argv[3]);        // 山札ファイル
  Disp_Mode = atoi(argv[4]);    // 表示モード

  //----  トライ回数の確認
  if ( Trial > GNUM ) {
    puts("ERROR : トライ回数が多すぎます。");
    exit(EXIT_FAILURE);
  }

  //----  山札ファイルのオープンとエラー処理
  fp = fopen(Deck, "r");
  if ( fp == NULL ) {
    puts("ERROR : 山札ファイルの指定が不正です。");
    exit(EXIT_FAILURE);
  }

  //----  ファイルの作成
  make_dir();    // ディレクトリ
  make_log();    // ログファイル

  //----  乱数の初期化
  srand(time(NULL));
  puts("");

  //----  トライ回数だけのゲームの反復
  total = 0.0;
  for ( k = 0; k < Trial; k++ ) {
    total += poker_exec(fp, point[k]);
  }

  //----  結果表の出力
  result_show(point);

  //----  平均点の出力
  ave = (double) total / Trial;
  my_printf(Result_Log, "\n");
  my_printf_d(Result_Log, "平均得点 :  %lf\n", ave);

  //----  ファイルのクローズ
  fclose(fp);
  fclose(Game_Log);
  fclose(Result_Log);

  //----  終了
  return 0;
}


//====================================================================
//  ポーカーの進行
//====================================================================

//--------------------------------------------------------------------
//  各トライの進行
//--------------------------------------------------------------------

double poker_exec(FILE *fp, int point[])
{
  //----  局所変数
  int deck[CNUM];    // 山札
  int used[CNUM];    // 捨札配列
  int us;            // 捨札数
  double try_pt;     // トライ得点
  int tk;            // テイク数

  //----  トライの準備
  card_deck(deck, fp);    // 山札の生成

  //----  トライの最中(テイクを繰り返す)
  us = 0;
  try_pt = 0.0;
  for ( tk = 0; tk < TAKE; tk++ ) {
    point[tk] = poker_take(deck, tk, used, &us);
    try_pt += point[tk] * Take_Weight[tk];
  }

  //----  トライ得点
  switch ( Disp_Mode ) {
  case 1:
    // トライ得点の表示(ログ保存付)
    fprintf(stdin, "トライ得点 : %6.2f\n", try_pt);
  case 0:
    // トライ得点のログへの保存
    fprintf(Game_Log, "トライ得点 : %6.2f\n", try_pt);
    break;
  }

  return try_pt;
}

//--------------------------------------------------------------------
//  テイクの実行
//--------------------------------------------------------------------

int poker_take(const int deck[], int tk, int used[], int *us)
{
  //----  局所変数
  int field[FNUM];            // 場札
  int hand[HNUM];             // 手札
  int state[CHNG+1][HNUM];    // 局面(手札の列)
  int ope[CHNG];              // 打手札
  int the;                    // 打手
  int cg;                     // チェンジ回数
  int take_p;                 // テイク得点
  int k;                      // 反復変数
  int field_cp[FNUM];         // 場札のコピー
  int hand_cp[HNUM];          // 手札のコピー
  int used_cp[CNUM];          // 捨札のコピー

  //----  テイクの準備
  if ( *us +5 < CNUM ) {
    arr_copy(hand, &deck[*us], HNUM);    // 手札の配布(5枚)
    arr_copy(state[0], hand, HNUM);      // 局面の記録
  } else {
    return 0;
  }

  //----  テイクの最中(チェンジの繰返し)
  for ( cg = 0; cg < CHNG; cg++ ) {
    //----  札のコピー
    if ( *us < CNUM-5 ) {
      arr_copy(hand_cp, hand, HNUM);
      arr_copy(field_cp, field, cg);
      arr_copy(used_cp, used, *us);
      //----  戦略により捨札を決定
      the = strategy(hand_cp, field_cp, cg, tk, used_cp, *us);
    } else {
      the = -1; break;
    }
    //----
    ope[cg] = hand[the];                  // 打手札の記録
    if ( the < 0 ) { break; }             // 手札を交換せずにテイク終了
    field[cg] = ope[cg];                  // 捨札を場札に追加
    used[(*us)++] = ope[cg];              // 捨札を捨札配列に追加
    hand[the] = deck[HNUM-1+(*us)];       // 手札を交換
    arr_copy(state[cg+1], hand, HNUM);    // 局面の記録
  }

  //----  テイクの後処理
  take_p = poker_point(hand);                 // テイク得点

  for ( k = 0; k < HNUM; k++ ) {
    used[*us] = hand[k];                      // 手札を捨札配列に追加
    (*us)++;
  }

  //----  テイクの経過表示
  switch ( Disp_Mode ) {
  case 1:
    take_show(state, ope, field, cg, take_p);    // テイクの表示
    //----  テイク素点の表示
    my_printf_i(Game_Log, "テイク素点 : %4d\n", take_p);
    my_printf_d(Game_Log, "テイク得点 : %6.2f\n", take_p*Take_Weight[tk]);
    break;
  case 0:
    take_log(Game_Log, state, ope, field, cg, take_p);   // テイクのログ
    //----  テイク素点のログへの保存
    fprintf(Game_Log, "テイク素点 : %3d\n", take_p);
    break;
  }

  return take_p;
}


//====================================================================
//  ゲームの表示
//====================================================================

//--------------------------------------------------------------------
//  テイクの表示
//--------------------------------------------------------------------

void take_show(int st[][HNUM], int ope[], int fd[], int cg, int tp)
{
  int k;
  int p;
  for ( k = 0; k <= cg; k++ ) {
    //----  手札の表示
    my_printf_i(Game_Log, "[%d] ", k);
    card_show(st[k], HNUM);
    card_show_log(Game_Log, st[k], HNUM);
    //----  捨札の表示
    if ( k < cg ) { my_printf_c(Game_Log, " > %s", card_str(ope[k])); }
    my_printf(Game_Log, "\n");
  }
}

//--------------------------------------------------------------------
//  テイクのログファイルへの書出し
//--------------------------------------------------------------------

void take_log(FILE *fp, int st[][HNUM], int ope[], int fd[], int cg, int tp)
{
  int k;
  int p;
  for ( k = 0; k <= cg; k++ ) {
    //----  手札の表示
    fprintf(Game_Log, "[%d] ", k);
    //card_show(st[k], HNUM);
    card_show_log(Game_Log, st[k], HNUM);
    //----  捨札の表示
    if ( k < cg ) { fprintf(Game_Log, " > %s", card_str(ope[k])); }
    fprintf(Game_Log, "\n");
  }
}

//--------------------------------------------------------------------
//  結果表の表示
//--------------------------------------------------------------------

void result_show(int point[][TAKE])
{
  int i, j, k;
  int deg[POINT_NUM][TAKE];    //
  int sum[POINT_NUM];          //
  double scr[TAKE] = {0.0};    //
  double total = 0.0;          //

  //----  配列の初期化
  for( k = POINT_MIN; k <= POINT_MAX; k++ ) {
    sum[k] = 0;
    for( j = 0; j < TAKE; j++ ) {
      deg[k][j] = 0;
    }
  }

  //----  役ごとの集計
  for ( i = 0; i < Trial; i++ ) {
    for ( j = 0; j < TAKE; j++ ) {
      switch( point[i][j] ) {
      case P9: deg[9][j]++; break;
      case P8: deg[8][j]++; break;
      case P7: deg[7][j]++; break;
      case P6: deg[6][j]++; break;
      case P5: deg[5][j]++; break;
      case P4: deg[4][j]++; break;
      case P3: deg[3][j]++; break;
      case P2: deg[2][j]++; break;
      case P1: deg[1][j]++; break;
      case P0: deg[0][j]++; break;
      default : break;
      }
    }
  }

  //----  全テイクでの役の総数
  for( k = POINT_MIN; k <= POINT_MAX; k++ ) {
    for( j = 0; j < TAKE; j++ ) {
      sum[k] += deg[k][j];
      scr[j] += deg[k][j] * Hand_Value[k];
    }
  }
  puts("");

  //----  結果表のテイク番号の表示
  my_printf(Result_Log, "        役名         ");
  for( i = 0; i < TAKE; i++ ) {
    my_printf_i(Result_Log, "| take%d", i+1);
  }
  my_printf(Result_Log, "|  合計  \n");
  //-----  結果表の区切線の表示
  my_printf(Result_Log, "---------------------");
  for( i = 0; i <= TAKE; i++ ) {
    my_printf(Result_Log, "+------");
  }
  my_printf(Result_Log, "\n");

  for ( i = POINT_MIN; i <= POINT_MAX; i++ ) {
    //----  結果表の役名の表示
    switch( i ) {
      case 9: my_printf(Result_Log, "ロイヤルストレート   "); break;
      case 8: my_printf(Result_Log, "ストレートフラッシュ "); break;
      case 7: my_printf(Result_Log, "フォーカード         "); break;
      case 6: my_printf(Result_Log, "フルハウス           "); break;
      case 5: my_printf(Result_Log, "フラッシュ           "); break;
      case 4: my_printf(Result_Log, "ストレート           "); break;
      case 3: my_printf(Result_Log, "スリーカード         "); break;
      case 2: my_printf(Result_Log, "ツーペア             "); break;
      case 1: my_printf(Result_Log, "ワンペア             "); break;
      case 0: my_printf(Result_Log, "ノーペア             "); break;
      default : break;
    }
    for ( j = 0; j < TAKE; j++ ) {
      //----  結果表の役頻度の表示
      my_printf_i(Result_Log, "|%6d", deg[i][j]);
      if( j == TAKE-1 ) {
        my_printf_i(Result_Log, "|%6d", sum[i]);
      }
    }
    my_printf(Result_Log, "\n");
  }

  //-----  結果表の区切線の表示
  my_printf(Result_Log, "---------------------");
  for( i = 0; i <= TAKE; i++ ) {
    my_printf(Result_Log, "+------");
  }
  my_printf(Result_Log, "\n");

  //----
  my_printf(Result_Log, "単純素点             ");
  for ( j = 0; j < TAKE; j++ ) {
    my_printf_d(Result_Log, "|%6.2f", scr[j] / Trial);
  }
  my_printf(Result_Log, "|\n");

  //----
  my_printf(Result_Log, "傾斜得点             ");
  for ( j = 0; j < TAKE; j++ ) {
    my_printf_d(Result_Log, "|%6.2f", scr[j] * Take_Weight[j]/Trial);
    total += scr[j] * Take_Weight[j];
  }
  my_printf_d(Result_Log, "|%6.2f\n", total / Trial);
}


//====================================================================
//  ポーカーの役と得点
//====================================================================

//--------------------------------------------------------------------
//  ポーカーの得点
//--------------------------------------------------------------------

int poker_point(int hand[])
{
  //----  局所宣言
  int num[13] = {0};     // 数位ごとの個数を格納する配列(A23456789TJQK)
  int sut[4] = {0};      // 種類ごとの個数を格納する配列(SHDC)
  int p0, p1, p2, p3;    // 最終および系別の役の得点
  int k;    // 反復変数
  int t;    // 一時変数

  //----  数位および種類ごとの計数
  for ( k = 0; k < HNUM; k++ ) { t = hand[k] % 13; num[t]++; }    // 数位
  for ( k = 0; k < HNUM; k++ ) { t = hand[k] / 13; sut[t]++; }    // 種類

  //----  系別の役の判定
  p1 = poker_point_pair(num);            // ペア系
  p2 = poker_point_flash(sut);           // フラッシュ系
  p3 = poker_point_straight(num, p2);    // ストレート系(ロイヤル判定のための引数付)

  //----  最高点の役を採用
  p0 = p1;                       // ペア系の役の得点
  if ( p0 < p2 ) { p0 = p2; }    // フラッシュ系の役への変更
  if ( p0 < p3 ) { p0 = p3; }    // ストレート系の役への変更

  //----  確定した役の返却
  return p0;
}

//--------------------------------------------------------------------
//  ペア判定
//--------------------------------------------------------------------

int poker_point_pair(int num[])
{
  //----  局所宣言
  int c2 = 0, c3 = 0;
  int k;

  //----  フォーカード
  for ( k = 0; k < 13; k++ ) { if ( num[k] == 4 ) { return P7; } }

  //----  その他のペア要素の検出
  for ( k = 0; k < 13; k++ ) {
    if ( num[k] == 3 ) { c3++; }
    if ( num[k] == 2 ) { c2++; }
  }

  //----  フルハウス
  if ( c3 == 1 && c2 == 1 ) { return P6; }
  //----  スリーカインズ
  if ( c3 == 1 ) { return P3; }
  //----  ツーペア
  if ( c2 == 2 ) { return P2; }
  //----  ワンペア
  if ( c2 == 1 ) { return P1; }
  //----  ノーペア
  return 0;
}

//--------------------------------------------------------------------
//  フラッシュ判定
//--------------------------------------------------------------------

int poker_point_flash(int sut[])
{
  int k;
  for ( k = 0; k < 4; k++ ) { if ( sut[k] == 5 ) { return P5; } }
  return P0;
}

//--------------------------------------------------------------------
//  ストレート判定
//--------------------------------------------------------------------

int poker_point_straight(int num[], int p)
{
  int len = 0;
  int k;
  for ( k = 0; k < 13; k++ ) {
    if ( num[k] > 0 ) {
      len++;
      if ( len == 5 ) { break; }
    } else { len = 0; }
  }

  //----  ロイヤルストレート(TJQKA)
  if ( len == 4 && num[0] == 1 ) { len = 6; }
  //----  ロイヤルストレートフラッシュ
  if ( len == 6 ) { if ( p > 0 ) { return P9; } else { return P4; } }
  //----  ノーマルストレート
  if ( len == 5 ) { if ( p > 0 ) { return P8; } else { return P4; } }

  //----
  return 0;
}


//====================================================================
//  ファイル処理
//====================================================================

//--------------------------------------------------------------------
//  ログディレクトリの作成
//--------------------------------------------------------------------

void make_dir(void)
{
  char command[COMM_LEN];         // システムコマンド
  DIR *log_dir;                   // ログディレクトリ

  //----  ログディレクトリへのパス
  strcpy(Dir, "../../log/");
  strcat(Dir, Name);

  //----  ログディレクトリの有無
  if ( (log_dir = opendir(Dir)) == NULL ) {
    //----  ログディレクトリの作成(mkdirコマンドの呼出)
    strcpy(command, "mkdir ");    // UNIX
    strcat(command, Dir);
    system(command);
  }
}

//--------------------------------------------------------------------
//  ログファイルの作成
//--------------------------------------------------------------------

void make_log(void)
{
  char path1[FILE_NAME_LEN];            // Game.txtへのパス
  char path2[FILE_NAME_LEN];            // Result.txtへのパス
  char path3[FILE_NAME_LEN];            // Deck.txtへのパス
  char command[COMM_LEN];               // システムコマンド

  //---- Game.txtへのパスを作成
  strcpy(path1, Dir);
  strcat(path1, "/Game.txt");

  //---- Result.txtへのパスを作成
  strcpy(path2, Dir);
  strcat(path2, "/Result.txt");

  //---- Deck.txtへのパスを作成
  strcpy(path3, Dir);
  strcat(path3, "/Deck.txt");

  //---- Game.txtのオープン
  if( (Game_Log = fopen(path1, "w")) == NULL ) {
    printf("Game.txtをオープンできない\n");
    exit(1);
  }

  //---- Result.txtのオープン
  if( (Result_Log = fopen(path2, "w")) == NULL ) {
    printf("Result.txtをオープンできない\n");
    exit(1);
  }

  //---- Deck.txtのコピー(cpコマンドの呼出)
  strcpy(command, "cp ");
  strcat(command, Deck);
  strcat(command, " ");
  strcat(command, path3);
  system(command);
}
