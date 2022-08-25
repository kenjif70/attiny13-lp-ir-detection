// 赤外線信号を検出している間、LED を点灯させたり、モーターを回したりするプログラム
//
// ウォッチドックタイマーを使い、定期的に赤外線の検出を行う。赤外線が検出されない場合はスリープし、消費電力を低減する。
// 赤外線受信モジュールも、検出を行っている時だけ通電することで、消費電力を低減する。
//
// ATtiny13a 用。https://github.com/MCUdude/MicroCore を使用
//
// より省電力にするために、プログラムの書き込み時は以下の設定を推奨
// - Clock: 1MHz (128kHz でも問題ないが、128kHz にするには Arduino ISP のコードを 1 行変更する必要がある)
// - BOD 無効
// 
// 使用するピン・ポート
// - PB2: 出力。LED などを接続する。赤外線が検出されている間は HIGH、それ以外は LOW になる
// - PB3: 入力。赤外線受信モジュールからの入力で、赤外線が検出されている間は LOW、それ以外は HIGH
// - PB4: 出力。赤外線受信モジュールに電源を供給する
//
// 参考: http://milkandlait.blogspot.com/2015/02/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// 予期しないタイミングでウォッチドックタイマーが起動しないように、初期化時にウォッチドッグを禁止
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));
void get_mcusr(void) {
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

// 空のウォッチドッグ割り込みハンドラを用意。ウォッチドックタイマー割り込みはスリープ解除のためだけに必要なので、割り込みハンドラは空で構わない
EMPTY_INTERRUPT(WDT_vect)

// 処理の概要
// - 0.25 鋲毎にウォッチドックタイマー割り込みを発生させ、スリープを解除するようにする
// - 割り込みが 4 回起きたら (つまり 1 秒毎に) 赤外線の検出を行う
// - 赤外線を 4 回連続で検出したら、普通の家電リモコンからの赤外線ではないと判断し、PB2 を HIGH にする
//   (家電のリモコンはボタンを押し続けない限り 1 秒も信号を送ることはない。ボタンを押し続けたとしても、ずっと赤外線が送られ続ける可能性は低い)
// - 赤外線が検出されなくなったら、PB2 を LOW にする
int main(void) {
  static bool ir_state = false; // 赤外線を検出したかを示す変数
  static int wdt_count = 0; // ウォッチドックタイマー割り込みが何回起きたかを示す変数
  static int ir_on_count = 0; // 何回連続で赤外線を検出したかを示す変数
  
  DDRB  = 0b11110111; // PORTB 3 を入力、それ以外を出力に設定
  PORTB = 0b00000000; // 出力は LOW にし、入力はプルアップしない (PORTB 3 は外部抵抗でプルアップする)

  ACSR |= _BV(ACD); // 省電力のため、アナログコンパレータを停止する
  
  // ウォッチドッグタイマ割り込みを有効にして(WDTIE=1)、ウォッチドッグリセットは発生しないようにする(WDE=0)
  // 割り込み周期は 0.25s
  cli();
  MCUSR = 0;
  WDTCR |= _BV(WDCE) | _BV(WDE);
  WDTCR = _BV(WDTIE) | _BV(WDP2);
  sei();

  // スリープ時、最も省電力となる POWER DOWN モードにする
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  while(1) {
    wdt_reset(); // ウォッチドッグタイマーの計測開始
    sleep_mode(); // スリープに入る
    
    // ウォッチドックの割り込みが起きたら、スリープが解除され、この次の行から実行が再開される
    
    WDTCR |= _BV(WDTIF); // 割り込みが発生したはずなので、WDTIF をリセット
    
    wdt_count++; // ウォッチドックの割り込みが起きたので、カウントアップ
    if (wdt_count > 3 || ir_on_count > 0) { // 4 回目の割り込みか、もしくは前回赤外線が検出されていたら、赤外線の検出を行う
      PORTB |= _BV(PB4); // 赤外線センサーの電源につなげた PB4 を HIGH にする (電源を入れる)
      _delay_ms(0.5); // センサーの出力が安定するまで 0.5ms 待つ
      ir_state = bit_is_clear(PINB, PORTB3); // 0 (clear) なら赤外線を検出している
      PORTB &= ~_BV(PB4); // 電源 OFF
  
      if (ir_state) {
        ir_on_count++; // 赤外線を検出していたら、カウントアップ
        if (ir_on_count > 3) { // 4 回連続で赤外線を検出していたら、カウントアップ
          PORTB |= _BV(PB2); // PB2 を HIGH にする
        }
      } else {
        // 赤外線を検出していないなら、モーターを止め、カウンタを 0 に戻す
        PORTB &= ~_BV(PB2); // PB2 を LOW にする
        ir_on_count = 0;
        wdt_count = 0;
      }
    }
  }
}
