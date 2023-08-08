#include "defines.h"
#include "kozos.h"
#include "memory.h"
#include "intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "lib.h"
#include "timer.h"

#define THREAD_NUM 6
#define PRIORITY_NUM 16
#define THREAD_NAME_SIZE 15

/* ����åɥ���ƥ����� */
typedef struct _kz_context {
  uint32 sp; /* �����å��ݥ��� */
} kz_context;

/* ����������ȥ���֥�å� */
typedef struct _kz_thread {
  struct _kz_thread *next;
  char name[THREAD_NAME_SIZE];
  int priority; /* ͥ���� */
  uint8 cpu_time;
  char *stack; /* �����å� */
  uint32 flags; /* �Ƽ�ե饰 */
#define KZ_THREAD_FLAG_READY (1<<0)

  struct { /* ����åɤΥ������ȥ��å�(thread_init())���Ϥ��ѥ�᡼�� */
    kz_func_t func; /* ����åɤΥᥤ��ؿ� */
    int argc;
    char **argv;
  } init;

  struct { /* �����ƥॳ�����ѥХåե� */
    kz_syscall_type_t type;
    kz_syscall_param_t *param;
  } syscall;

  kz_context context;
} kz_thread;

/* ����åɤΥ�ǥ������塼 */
static struct {
  kz_thread *head;
  kz_thread *tail;
} readyque[PRIORITY_NUM];

/* ��̲���� */
static struct {
  kz_thread *thread;
  int wakeup_time;
} sleep_thread[THREAD_NUM];

static kz_thread *current;
static kz_thread threads[THREAD_NUM];
static kz_handler_t handlers[SOFTVEC_TYPE_NUM]; /* ����ߥϥ�ɥ� */

static uint8 clock_time = 0;
static uint8 time = 0;

void print_readyque(int priority)
{
  kz_thread *thp = readyque[priority].head;

  puts("priority: ");
  putxval(priority, 1);
  puts("\n");
  for(; thp; thp = thp->next) {
    puts(thp->name);
    puts("\n");
  }
}

void dispatch(kz_context *context);

/* �����ȥ���åɤ��ǥ������塼����ȴ���Ф� */
static int getcurrent(void)
{
  puts("getcurrent\n");
  if(current == NULL) {
    puts("����current��null�Ǥ�\n");
    return -1;
  }
  if(!(current->flags & KZ_THREAD_FLAG_READY)) {
    /* ���Ǥ˥�ǥ������֤Ǥʤ�����̵�� */
    return 1;
  }

  /* �����ȥ���åɤ�ɬ����Ƭ�ˤ���Ϥ��ʤΤǡ���Ƭ����ȴ���Ф� */
  
  readyque[current->priority].head = current->next;
  if(readyque[current->priority].head == NULL) {
    readyque[current->priority].tail = NULL;
  }
  current->flags &= ~KZ_THREAD_FLAG_READY;
  current->next = NULL;

  return 0;
}

/* �����ȥ���åɤ��ǥ������塼�ˤĤʤ��� */
static int putcurrent(void)
{
  puts("putcurrent\n");
  if(current == NULL) {
    puts("putcurrent: ����current��null�Ǥ�\n");
    return -1;
  }
  if(current->flags & KZ_THREAD_FLAG_READY) {
    /* ���Ǥ˥�ǥ������֤ʤ鲿�⤷�ʤ� */
    return 1;
  }

  /* ��ǥ������塼����������³���� */
  if(readyque[current->priority].tail) {
    readyque[current->priority].tail->next = current;
  } else {
    readyque[current->priority].head = current;
  }
  readyque[current->priority].tail = current;
  current->flags |= KZ_THREAD_FLAG_READY;

  return 0;
}

static void thread_end(void)
{
  puts("thread_end\n");
  kz_exit();
}

/* ����åɤΥ������ȥ��å� */
static void thread_init(kz_thread *thp)
{
  puts("thread_init\n");
  /* ����åɤΥᥤ��ؿ���ƤӽФ� */
  thp->init.func(thp->init.argc, thp->init.argv);
  thread_end();
}

/* �����ƥॳ����ν���(kz_run():����åɤε�ư) */
static kz_thread_id_t thread_run(kz_func_t func, char *name, int priority,
                                 int stacksize, int argc, char *argv[])
{
  puts("thread_run\n");
  int i;
  kz_thread *thp;
  uint32 *sp;
  extern char userstack;
  static char *thread_stack = &userstack; /* 0xfff40 */

  /* �����Ƥ��륿��������ȥ���֥�å��򸡺� */
  for(i = 0; i < THREAD_NUM; i++) {
    thp = &threads[i];
    if(!thp->init.func) /* ���Ĥ��ä� */
      break;
  }
  if(i == THREAD_NUM) /* ���Ĥ���ʤ��ä� */
    return -1;

  memset(thp, 0, sizeof(*thp));

  /* ����������ȥ���֥�å������� */
  strcpy(thp->name, name);
  thp->next      = NULL;
  thp->priority  = priority;
  thp->flags     = 0;

  thp->init.func = func;
  thp->init.argc = argc;
  thp->init.argv = argv;

  /* �����å��ΰ����� */
  memset(thread_stack, 0, stacksize);
  thread_stack += stacksize;

  thp->stack = thread_stack; /* �����å������� */

  /* �����å��ν���� */
  sp = (uint32*)thp->stack;
  *(--sp) = (uint32)thread_end;

  /**
   * �ץ���५���󥿤����ꤹ��
   * ����åɤ�ͥ���٤�����ξ��ˤϡ�����߶ػߥ���åɤȤ��롥
  */
  *(--sp) = (uint32)thread_init | ((uint32)(priority ? 0 : 0xc0) << 24);

  *(--sp) = 0; /* ER6 */
  *(--sp) = 0; /* ER5 */
  *(--sp) = 0; /* ER4 */
  *(--sp) = 0; /* ER3 */
  *(--sp) = 0; /* ER2 */
  *(--sp) = 0; /* ER1 */

  *(--sp) = (uint32)thp; /* ER0 */

  /* ����åɤΥ���ƥ����Ȥ����� */
  thp->context.sp = (uint32)sp;

  /* �����ƥॳ�����ƤӽФ�������åɤ��ǥ������塼���᤹ */
  putcurrent();

  /* ����������������åɤ򡢥�ǥ������塼����³���� */
  current = thp;
  putcurrent();

  return (kz_thread_id_t)current;
}

/* �����ƥॳ����ν���(kz_exit():����åɤν�λ) */
static int thread_exit(void)
{
  puts("thread_exit\n");
  puts(current->name);
  puts(" EXIT.\n");
  memset(current, 0, sizeof(*current));
  return 0;
}

/* �����ƥॳ����ν���(kz_wait():����åɤμ¹Ը�����) */
static int thread_wait(void)
{
  puts("thread_wait\n");
  putcurrent();
  return 0;
}

/* �����ƥॳ����ν���(kz_sleep():s�å���åɤ򥹥꡼��) */
static int thread_sleep(int s)
{
  puts("thread_sleep\n");

  int i;
  for(i = 0; i < THREAD_NUM; i++) {
    if(sleep_thread[i].thread == NULL) /* ���Ĥ��ä� */
      break;
  }
  if(i == THREAD_NUM) /* ���Ĥ���ʤ��ä� */
    return -1;

  sleep_thread[i].thread = current;
  sleep_thread[i].wakeup_time = time + s;

  return 0;
}

/* �����ƥॳ����ν���(kz_wakeup():����åɤΥ����������å�) */
static int thread_wakeup(kz_thread_id_t id)
{
  puts("thread_wakeup\n");
  putcurrent();

  current = (kz_thread*)id;
  putcurrent();

  return 0;
}

/* �����ƥॳ����ν���(kz_getid():����å�ID����) */
static kz_thread_id_t thread_getid(void)
{
  puts("thread_getid\n");
  putcurrent();
  return (kz_thread_id_t)current;
}

/* �����ƥॳ����ν���(kz_chpri():����åɤ�ͥ�����ѹ�) */
static int thread_chpri(int priority)
{
  puts("thread_chpri\n");
  int old = current->priority;
  if(priority >= 0)
    current->priority = priority; /* ͥ�����ѹ� */
  putcurrent();
  return old;
}

// /* �����ƥॳ����ν���(kz_kmalloc():ưŪ�������) */
// static void *thread_kmalloc(int size)
// {
//   puts("thread_kmalloc\n");
//   putcurrent();
//   return kzmem_alloc(size);
// }

/* �����ƥॳ����ν���(kz_ksbrk():ưŪ�������) */
static void *thread_ksbrk(int size)
{
  puts("thread_ksbrk\n");
  putcurrent();
  return kzmem_alloc(size);
}

/* ����ߥϥ�ɥ����Ͽ */
static int setintr(softvec_type_t type, kz_handler_t handler)
{
  static void thread_intr(softvec_type_t type, unsigned long sp);

  /**
   * ����ߤ�����դ��뤿��ˡ����եȥ����������ߥ٥�����
   * OS�γ����߽�����������Ȥʤ�ؿ�����Ͽ����
  */
  softvec_setintr(type, thread_intr);

  handlers[type] = handler;

  return 0;
}

static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  puts("call_functions\n");
  /* �����ƥॳ����μ¹����current�����Ѥ��Τ���� */
  switch(type) {
    case KZ_SYSCALL_TYPE_RUN: /* kz_run() */
      p->un.run.ret = thread_run(p->un.run.func, p->un.run.name,
                                 p->un.run.priority, p->un.run.stacksize,
                                 p->un.run.argc, p->un.run.argv);
      break;
    case KZ_SYSCALL_TYPE_EXIT:
      /* TCB���õ���Τǡ�����ͤ�񤭹���ǤϤ����ʤ� */
      thread_exit();
      break;
    case KZ_SYSCALL_TYPE_WAIT:
      p->un.wait.ret = thread_wait();
      break;
    case KZ_SYSCALL_TYPE_SLEEP:
      p->un.sleep.ret = thread_sleep(p->un.sleep.ms);
      break;
    case KZ_SYSCALL_TYPE_WAKEUP:
      p->un.wakeup.ret = thread_wakeup(p->un.wakeup.id);
      break;
    case KZ_SYSCALL_TYPE_GETID:
      p->un.getid.ret = thread_getid();
      break;
    case KZ_SYSCALL_TYPE_CHPRI:
      p->un.chpri.ret = thread_chpri(p->un.chpri.priority);
      break;
    case KZ_SYSCALL_TYPE_KSBRK:
      p->un.ksbrk.ret = thread_ksbrk(p->un.ksbrk.size);
    default:
      break;
  }
}

/* �����ƥॳ����ν��� */
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  puts("syscall_proc\n");
  getcurrent();
  call_functions(type, p);
}

/* ����åɤΥ������塼��� */
static void schedule(void)
{
  puts("schedule ");
  int i;

  /**
   * ͥ���٤ι⤤��(ͥ���٤ο��ͤξ�������)�˥�ǥ������塼�򸫤ơ�
   * ư���ǽ�ʥ���åɤ򸡺����롥
  */
  for(i = 0; i < PRIORITY_NUM; i++) {
    if(readyque[i].head) /* ���Ĥ��ä� */
      break;
  }
  if(i == PRIORITY_NUM) /* ���Ĥ���ʤ��ä� */
    kz_sysdown();

  current = readyque[i].head; /* �����ȥ���åɤ����ꤹ�� */
  puts(current->name);
  puts("\n");
}

static void syscall_intr(void)
{
  puts("syscall_intr\n");
  syscall_proc(current->syscall.type, current->syscall.param);
}

static void softerr_intr(void)
{
  puts("softerr_intr\n");
  puts(current->name);
  puts(" DOWN.\n");
  getcurrent(); /* ��ǥ������塼���鳰�� */
  thread_exit();
}

/* �����߽����������ؿ� */
static void thread_intr(softvec_type_t type, unsigned long sp)
{
  puts("thread_intr\n");
  /* �����ȥ���åɤΥ���ƥ����Ȥ���¸���� */
  current->context.sp = sp;

  /**
   * �����ߤ��Ȥν�����¹Ԥ���
   * SOFTVEC_TYPE_SYSCALL, SOFTVEC_TYPE_SOFTERR�ξ���
   * syscall_intr(), softerr_intr()���ϥ�ɥ����Ͽ����Ƥ���Τǡ�
   * ����餬�¹Ԥ���롥
  */
  if(handlers[type])
    handlers[type]();

  schedule();

  puts("dispatch\n");
  dispatch(&current->context);
  /* �����ˤ��֤äƤ��ʤ� */
}

/* �����߽����������ؿ� */
static void timer_intr(softvec_type_t type, unsigned long sp) {
  puts("timer_intr: current->cpu_time: ");
  putxval(current->cpu_time, 3);
  puts("\n");

  /* �����ȥ���åɤΥ���ƥ����Ȥ���¸���� */
  current->context.sp = sp;
  
  /* �����޷�³���� */
  timer8_clear_flag(0);
  
  /* 1�ä��Ȥν��� */
  if(++clock_time >= TIMER_HZ) {
    time++; /* ����򹹿� */

    clock_time -= TIMER_HZ;

    int i;
    for(i = 0; i < THREAD_NUM; i++) {
      if(sleep_thread[i].thread == NULL)
        continue;
      
      if(sleep_thread[i].wakeup_time == time) {
        kz_thread *tmp = current;
        current = sleep_thread[i].thread;
        putcurrent();

        current = tmp;

        sleep_thread[i].thread = NULL;
      }
    }
  }

  // �����ॹ�饤������
  current->cpu_time++;
  if(current->cpu_time >= (uint8)(TIMER_HZ * (CPU_QUANTUM_MS / 1000))) {

    getcurrent();
    putcurrent();

    // print_readyque(3);

    current->cpu_time = 0;

    schedule();
    puts("dispatch\n");
    dispatch(&current->context);
    /* �����ˤ��֤äƤ��ʤ� */
  }
}

void kz_start(kz_func_t func, char *name, int priority, int stacksize,
              int argc, char *argv[])
{
  puts("kz_start\n");


  current = NULL;

  memset(readyque, 0, sizeof(readyque));
  memset(threads, 0, sizeof(threads));
  memset(handlers, 0, sizeof(handlers));

  /* ����ߥϥ�ɥ����Ͽ */
  setintr(SOFTVEC_TYPE_SYSCALL, syscall_intr);
  setintr(SOFTVEC_TYPE_SOFTERR, softerr_intr);

  softvec_setintr(SOFTVEC_TYPE_TIMINTR, timer_intr);
  timer8_hz(0);
  
  /* �����ƥॳ����ȯ���ԲĤʤΤ�ľ�ܴؿ���ƤӽФ���ưŪ������� */
  thread_ksbrk(INIT_MEMORY_SIZE);

  /* �����ƥॳ����ȯ���ԲĤʤΤ�ľ�ܴؿ���ƤӽФ��ƥ���åɤ�������� */
  current = (kz_thread*)thread_run(func, name, priority, stacksize, argc, argv);


  /* �ǽ�Υ���åɤ�ư */
  puts("dispatch\n");
  dispatch(&current->context);

  /* �����ˤ��֤äƤ��ʤ� */
}

void kz_sysdown(void)
{
  puts("system error!\n");
  timer8_stop(0);
  while(1)
    ;
}

/* �����ƥॳ����ƤӽФ��ѥ饤�֥��ؿ� */
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
  puts("kz_syscall\n");
  current->syscall.type  = type;
  current->syscall.param = param;
  asm volatile ("trapa #0"); /* �ȥ�å׳����ȯ��(intr_syscall) */
}