#include "h8-3052-iodef.h"
#include "lcd.h"

#define KEYBUFSIZE 10  /* キーバッファの大きさ */
#define KEYCHKCOUNT 5  /* キーの連続状態を調べるバッファ上の長さ　 */
                         /* ↑キーバッファの大きさよりも小さくすること */
                         /*   余裕が少ないと正しく読めないことがある */
#define KEYROWNUM 4    /* キー配列の列数(縦に並んでいる個数) */
#define KEYCOLNUM 3    /* キー配列の行数(横に並んでいる個数) */
#define KEYMINNUM  1   /* キー番号の最小値 */
#define KEYMAXNUM 12   /* キー番号の最大値 */
#define KEYNONE   -1   /* 指定したキーがない */
#define KEYOFF     0   /* 指定したキーはずっと離されている状態 */
#define KEYON      1   /* 指定したキーはずっと押されている状態 */
#define KEYTRANS   2   /* 指定したキーは遷移状態 */

// キースキャンを行って、状態を調べる関数群
// 一定時間（数ms程度）毎に keysense() を呼び出すことが前提
// 任意のキー状態を読み出すには key_check() を呼び出す

/* タイマ割り込み処理のため, バッファ関連は大域変数として確保 */
/* これらの変数は key.c 内のみで使用されている               */
int keybufdp; /* キーバッファ参照ポインタ */
unsigned char keybuf[KEYBUFSIZE][KEYROWNUM]; /* キーバッファ */

void key_init(void);
void key_sense(void);
int key_check(int keynum);

void key_init(void)
     /* キーを読み出すために必要な初期化を行う関数 */
     /* PA4-6 が LCD と関連するが, 対策済み       */
{
  int i,j;

  PADR = 0x0f;       /* PA0-3 は0アクティブ, PA4-6 は1アクティブ */
  PADDR = 0x7f;      /* PA0-3 はキーボードマトリクスの出力用 */
                     /* PA4-6 はLCD制御(E,R/W,RS)の出力用 */
  P6DDR = 0;         /* P60-2 はキーボードマトリクスの入力用 */
                     /* P63-6 はCPUのバス制御として固定(モード6の時) */
  keybufdp = 0;
  /* キーバッファのクリア */
  for (i = 0; i < KEYBUFSIZE; i++){
    for (j = 0; j < KEYROWNUM; j++){
      /* ここで何もキーが押されていない状態にバッファ(keybufdp)を初期化 */
      /* キーが押されていないときにビットが1となることに注意すること */
		keybuf[i][j] = 0xff;
    }
  }
}

void key_sense(void)
     /* キースキャンしてキーバッファに入れる関数          */
     /*   数ms 程度に一度, タイマ割り込み等で呼び出すこと */
     /*   大域変数 keybuf はキーデータを格納するバッファ  */
{
  /* リングバッファポインタ制御 */
  /* ここにバッファポインタ(keybufdp)の更新を書く */
  /* 　・バッファポインタが最新のスキャンデータを指すようにすること */
  /* 　・リングバッファのつなぎ目の処理を忘れないこと */
	keybufdp++;
	keybufdp %= KEYBUFSIZE;
  /* キースキャン */
  /* ここでキー列ごとにキースキャンしたデータをそのままキーバッファに格納する */
  /* キー列番号は、0:1〜3の列、1:4〜6の列、2:7〜9の列、3:*〜#の列 とする */
  /* 各キー列のキーデータは keybuf[バッファポインタ][キー列番号] に格納する */
  /* 　・PA0〜PA3だけを書き換えるように注意すること(他のビットの変化禁止) */
  /* 　・P60〜P62だけを読むように注意すること(他のビットは0にする) */
    //key 1,2,3
    //PADR = 0x07; // PA3 = L
	//keybuf[keybufdp][0] = ~(~P6DR & 0x07);   // データ入力
      
    //key 4,5,6
    //PADR = 0x0b;
	//keybuf[keybufdp][1] = ~(~P6DR & 0x07);   // データ入力
      
    //key 7,8,9
    //PADR = 0x0d;
	//keybuf[keybufdp][2] = ~(~P6DR & 0x07);   // データ入力
      
    //key *,0,#
    PADR = 0x0e;
	keybuf[keybufdp][3] = ~(~P6DR & 0x07);   // データ入力
}

int key_check(int keynum)
     /* キー番号を引数で与えると, キーの状態を調べて返す関数                 */
     /* キー番号(keynum)は 1-12 で指定(回路図の sw1-sw12 に対応)            */
     /* 基板上の 1-9 のキーは sw1-sw9 に対応している                        */
     /* 基板上の *,0,# のキーは sw10,sw11,sw12 にそれぞれ対応している       */
     /* 戻り値は, KEYOFF, KEYON, KEYTRANS, KEYNONE のいずれか              */
     /* チェック中の割り込みによるバッファ書き換え対策はバッファの大きさで対応 */
{
  int r;
  int i;
  int count = 0;
  int tmp;
  /* 最初にキー番号の範囲をチェックする */
  if ((keynum < 1) || (keynum > KEYMAXNUM))
    r = KEYNONE; /* キー番号指定が正しくないときはKEYNONEを返す */
  else {
    /* ここでキー番号からキー列番号とデータのビット位置を求める */
    /* キー列番号がわかると配列の参照ができる */
    /* データのビット位置がわかれば、指定されたキーのON/OFFがわかる */
    /* ここで宣言された長さ(KEYCHKCOUNT)分だけキーの状態を調べる */
    /* 　・リングバッファのつなぎ目の処理を忘れないこと */
    /* 　・途中でキースキャン割り込みが生じても矛盾しない処理を行うこと */
    /* 指定キーが全てONならKEYON、全てOFFならKEYOFF、それ以外はKEYTRANS とする*/
	tmp =  keybufdp;

	for(i = 0; i<KEYCHKCOUNT; i++){
		if(~(keybuf[(tmp - i + KEYBUFSIZE)%KEYBUFSIZE][(keynum-1)/3]) & 0x01<<((keynum-1)%3)) count++;
	}
  	//lcd_cursor(15,0);                     /* LCD にメッセージ表示 */
   	//lcd_printstr("0" + count);

	if(count == 5){
		r = KEYON;
	}else if(count == 0){
		r = KEYOFF;
	}else{
		r = KEYTRANS;
	}

  }
  return r;
}
