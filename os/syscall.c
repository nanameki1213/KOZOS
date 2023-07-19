#include "defines.h"
#include "kozos.h"
#include "syscall.h"
#include "timer.h"

/* システム・コール */

kz_thread_id_t kz_run(kz_func_t func, char *name, int priority, int stacksize,
		      int argc, char *argv[])
{
  puts("kz_run\n");
  kz_syscall_param_t param;
  param.un.run.func = func;
  param.un.run.name = name;
  param.un.run.priority = priority;
  param.un.run.stacksize = stacksize;
  param.un.run.argc = argc;
  param.un.run.argv = argv;
  kz_syscall(KZ_SYSCALL_TYPE_RUN, &param);
  return param.un.run.ret;
}

void kz_exit(void)
{
  puts("kz_exit\n");
  kz_syscall(KZ_SYSCALL_TYPE_EXIT, NULL);
}

int kz_wait(void)
{
  puts("kz_wait\n");
  kz_syscall_param_t param;
  kz_syscall(KZ_SYSCALL_TYPE_WAIT, &param);
  return param.un.wait.ret;
}

int kz_sleep(int ms)
{
  puts("kz_sleep\n");
  kz_syscall_param_t param;
  param.un.sleep.ms = ms;
  kz_syscall(KZ_SYSCALL_TYPE_SLEEP, &param);
  return param.un.sleep.ret;
}

int kz_wakeup(kz_thread_id_t id)
{
  puts("kz_wakeup\n");
  kz_syscall_param_t param;
  param.un.wakeup.id = id;
  kz_syscall(KZ_SYSCALL_TYPE_WAKEUP, &param);
  return param.un.wakeup.ret;
}

kz_thread_id_t kz_getid(void)
{
  puts("kz_getid\n");
  kz_syscall_param_t param;
  kz_syscall(KZ_SYSCALL_TYPE_GETID, &param);
  return param.un.getid.ret;
}

int kz_chpri(int priority)
{
  puts("kz_chpri\n");
  kz_syscall_param_t param;
  param.un.chpri.priority = priority;
  kz_syscall(KZ_SYSCALL_TYPE_CHPRI, &param);
  return param.un.chpri.ret;
}