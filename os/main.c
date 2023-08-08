#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"
#include "timer.h"

kz_thread_id_t test_1_id;
kz_thread_id_t test_2_id;
kz_thread_id_t test_3_id;

/* システム・タスクとユーザ・スレッドの起動 */
static int start_threads(int argc, char *argv[])
{
  test_1_id = kz_run(test_1_main, "test_1_main", 3, 0x100, 0, NULL);
  test_2_id = kz_run(test_2_main, "test_2_main", 3, 0x100, 0, NULL);
  test_3_id = kz_run(test_3_main, "test_3_main", 3, 0x100, 0, NULL);
  kz_chpri(15);
  INTR_ENABLE;
  timer8_stop(0);
  while(1) {
    asm volatile ("sleep");
  }
  return 0;
}

int main(void)
{
  INTR_DISABLE; /* 割込み無効にする */

  puts("kozos boot succeed!\n");

  /* OSの動作開始 */
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  /* ここには戻ってこない */

  return 0;
}
