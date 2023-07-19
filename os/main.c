#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "lib.h"

kz_thread_id_t test09_1_id;
kz_thread_id_t test09_2_id;
kz_thread_id_t test09_3_id;

/* �����ƥࡦ�������ȥ桼��������åɤε�ư */
static int start_threads(int argc, char *argv[])
{
  test09_1_id = kz_run(test09_1_main, "test09_1_main", 3, 0x100, 0, NULL);
  test09_2_id = kz_run(test09_2_main, "test09_2_main", 3, 0x100, 0, NULL);
  test09_3_id = kz_run(test09_3_main, "test09_3_main", 3, 0x100, 0, NULL);
  // timer8_start(0, 300);
  kz_chpri(15);
  INTR_ENABLE;
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
