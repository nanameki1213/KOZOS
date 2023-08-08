#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"
#include "timer.h"

kz_thread_id_t test_1_id;
kz_thread_id_t test_2_id;
kz_thread_id_t test_3_id;

/* �����ƥࡦ�������ȥ桼��������åɤε�ư */
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
  INTR_DISABLE; /* �����̵���ˤ��� */

  puts("kozos boot succeed!\n");

  /* OS��ư��� */
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  /* �����ˤ���äƤ��ʤ� */

  return 0;
}
