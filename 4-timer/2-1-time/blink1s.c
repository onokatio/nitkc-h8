#include "h8-3052-iodef.h"
#include "h8-3052-int.h"
#include "timer.h"
#include "lcd.h"

/* 割り込みの周期 1000us */
#define INT1MS 1000

/* 割り込み処理に必要な変数は大域変数にとる */
/* volatile はプログラムの流れとは無関係に変化する変数に必ずつける */
volatile unsigned int sys_time;
volatile unsigned int sec_time; /* 1秒毎に +1 される変数を確保 */
volatile unsigned int old_time;

/* 割り込みハンドラのプロトタイプ宣言 */
void int_imia0(void);

int main(void)
{
  ROMEMU(); /* ROMエミュレーションをON (割り込み使用時必須) */
  lcd_init();

  /* 時間を格納する変数の初期化 */
  sys_time = 0;
  sec_time = 0;
  old_time = 0;

  timer_init();        /* タイマを使用前に初期化する */
  timer_set(0,INT1MS); /* タイマ0を1ms間隔で動作設定 */
  timer_start(0);      /* タイマ(チャネル0)スタート  */

  ENINT();             /* CPU割り込み許可 */

  while (1) {
    /* 割り込み動作以外はこの無限ループを実行している */

	if (old_time != sec_time){
  		lcd_cursor(0,0);
		printnum(sec_time);
		old_time = sec_time;
	}

  }
  return 1; /* mainからの戻り値はエラーレベルを表す 0:正常終了 */
            /* 永久ループの外なので,戻ったら何かおかしい       */
}

void printnum(unsigned int num){
	if (num != 0){
		printnum(num/10);
	}else{
		return;
	}

	lcd_printch((num % 10) + '0');
}

#pragma interrupt
void int_imia0(void)
     /*
      *  タイマ0の割り込みハンドラ
      *    タイマ0 から割り込み要求がくると，この関数が呼び出される．
      *  タイマ0 の割り込みの場合は，この関数の名前（int_imia0）と
      *  決まっている．
      *    関数の直前に割り込みハンドラ指定の #pragma interrupt が必要．
      */
{
  /* 割り込みハンドラの処理が軽くなるように配慮すること */
  /* 外でできる処理はここには書かない */

  /* sys_time の更新 ( +1 する) をここに書く */
	sys_time++;

  
  /* ここに sec_time の更新 ( +1 する) を書く */
	if (sys_time % 1000 == 0){
		sec_time++;
	}
  
  /* 再びタイマ割り込みを使用するために必要な操作      */
  /*   タイマ0の割り込みフラグをクリアしないといけない */
  timer_intflag_reset(0);

  ENINT();       /* CPUを割り込み許可状態に */
}
