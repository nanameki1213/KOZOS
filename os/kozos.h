#ifndef _KOZOS_H_INCLUDED_
#define _KOZOS_H_INCLUDED_

#include "defines.h"
#include "syscall.h"

#define CPU_TIME 1
#define INIT_MEMORY_SIZE 0x40

/* �����ƥࡦ������ */
kz_thread_id_t kz_run(kz_func_t func, char *name, int priority, int stacksize,
		      int argc, char *argv[]);
void kz_exit(void);
int kz_wait(void);
int kz_sleep(int ms);
int kz_wakeup(kz_thread_id_t id);
kz_thread_id_t kz_getid(void);
int kz_chpri(int priority);
void *kz_kmalloc(int size);
int kz_kmkfree(void *p);
void *kz_sbrk(int size);

/* �饤�֥��ؿ� */
void kz_start(kz_func_t func, char *name, int priority, int stacksize,
	      int argc, char *argv[]);
void kz_sysdown(void);
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param);

// void kz_sleep(int ms);

/* �桼��������å�? */
int test_1_main(int argc, char *argv[]);
int test_2_main(int argc, char *arg[]);
int test_3_main(int argc, char *argv[]);
extern kz_thread_id_t test_1_id;
extern kz_thread_id_t test_2_id;
extern kz_thread_id_t test_3_id;

#endif
