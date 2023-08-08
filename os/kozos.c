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

/* スレッドコンテキスト */
typedef struct _kz_context {
  uint32 sp; /* スタックポインタ */
} kz_context;

/* タスクコントロールブロック */
typedef struct _kz_thread {
  struct _kz_thread *next;
  char name[THREAD_NAME_SIZE];
  int priority; /* 優先度 */
  uint8 cpu_time;
  char *stack; /* スタック */
  uint32 flags; /* 各種フラグ */
#define KZ_THREAD_FLAG_READY (1<<0)

  struct { /* スレッドのスタートアップ(thread_init())に渡すパラメータ */
    kz_func_t func; /* スレッドのメイン関数 */
    int argc;
    char **argv;
  } init;

  struct { /* システムコール用バッファ */
    kz_syscall_type_t type;
    kz_syscall_param_t *param;
  } syscall;

  kz_context context;
} kz_thread;

/* スレッドのレディーキュー */
static struct {
  kz_thread *head;
  kz_thread *tail;
} readyque[PRIORITY_NUM];

/* 休眠状態 */
static struct {
  kz_thread *thread;
  int wakeup_time;
} sleep_thread[THREAD_NUM];

static kz_thread *current;
static kz_thread threads[THREAD_NUM];
static kz_handler_t handlers[SOFTVEC_TYPE_NUM]; /* 割込みハンドラ */

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

/* カレントスレッドをレディーキューから抜き出す */
static int getcurrent(void)
{
  puts("getcurrent\n");
  if(current == NULL) {
    puts("現在currentはnullです\n");
    return -1;
  }
  if(!(current->flags & KZ_THREAD_FLAG_READY)) {
    /* すでにレディー状態でない場合は無視 */
    return 1;
  }

  /* カレントスレッドは必ず先頭にあるはずなので、先頭から抜き出す */
  
  readyque[current->priority].head = current->next;
  if(readyque[current->priority].head == NULL) {
    readyque[current->priority].tail = NULL;
  }
  current->flags &= ~KZ_THREAD_FLAG_READY;
  current->next = NULL;

  return 0;
}

/* カレントスレッドをレディーキューにつなげる */
static int putcurrent(void)
{
  puts("putcurrent\n");
  if(current == NULL) {
    puts("putcurrent: 現在currentはnullです\n");
    return -1;
  }
  if(current->flags & KZ_THREAD_FLAG_READY) {
    /* すでにレディー状態なら何もしない */
    return 1;
  }

  /* レディーキューの末尾に接続する */
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

/* スレッドのスタートアップ */
static void thread_init(kz_thread *thp)
{
  puts("thread_init\n");
  /* スレッドのメイン関数を呼び出す */
  thp->init.func(thp->init.argc, thp->init.argv);
  thread_end();
}

/* システムコールの処理(kz_run():スレッドの起動) */
static kz_thread_id_t thread_run(kz_func_t func, char *name, int priority,
                                 int stacksize, int argc, char *argv[])
{
  puts("thread_run\n");
  int i;
  kz_thread *thp;
  uint32 *sp;
  extern char userstack;
  static char *thread_stack = &userstack; /* 0xfff40 */

  /* 空いているタスクコントロールブロックを検索 */
  for(i = 0; i < THREAD_NUM; i++) {
    thp = &threads[i];
    if(!thp->init.func) /* 見つかった */
      break;
  }
  if(i == THREAD_NUM) /* 見つからなかった */
    return -1;

  memset(thp, 0, sizeof(*thp));

  /* タスクコントロールブロックの設定 */
  strcpy(thp->name, name);
  thp->next      = NULL;
  thp->priority  = priority;
  thp->flags     = 0;

  thp->init.func = func;
  thp->init.argc = argc;
  thp->init.argv = argv;

  /* スタック領域を獲得 */
  memset(thread_stack, 0, stacksize);
  thread_stack += stacksize;

  thp->stack = thread_stack; /* スタックを設定 */

  /* スタックの初期化 */
  sp = (uint32*)thp->stack;
  *(--sp) = (uint32)thread_end;

  /**
   * プログラムカウンタを設定する
   * スレッドの優先度がゼロの場合には，割込み禁止スレッドとする．
  */
  *(--sp) = (uint32)thread_init | ((uint32)(priority ? 0 : 0xc0) << 24);

  *(--sp) = 0; /* ER6 */
  *(--sp) = 0; /* ER5 */
  *(--sp) = 0; /* ER4 */
  *(--sp) = 0; /* ER3 */
  *(--sp) = 0; /* ER2 */
  *(--sp) = 0; /* ER1 */

  *(--sp) = (uint32)thp; /* ER0 */

  /* スレッドのコンテキストを設定 */
  thp->context.sp = (uint32)sp;

  /* システムコールを呼び出したスレッドをレディーキューに戻す */
  putcurrent();

  /* 新規作成したスレッドを、レディーキューに接続する */
  current = thp;
  putcurrent();

  return (kz_thread_id_t)current;
}

/* システムコールの処理(kz_exit():スレッドの終了) */
static int thread_exit(void)
{
  puts("thread_exit\n");
  puts(current->name);
  puts(" EXIT.\n");
  memset(current, 0, sizeof(*current));
  return 0;
}

/* システムコールの処理(kz_wait():スレッドの実行権放棄) */
static int thread_wait(void)
{
  puts("thread_wait\n");
  putcurrent();
  return 0;
}

/* システムコールの処理(kz_sleep():s秒スレッドをスリープ) */
static int thread_sleep(int s)
{
  puts("thread_sleep\n");

  int i;
  for(i = 0; i < THREAD_NUM; i++) {
    if(sleep_thread[i].thread == NULL) /* 見つかった */
      break;
  }
  if(i == THREAD_NUM) /* 見つからなかった */
    return -1;

  sleep_thread[i].thread = current;
  sleep_thread[i].wakeup_time = time + s;

  return 0;
}

/* システムコールの処理(kz_wakeup():スレッドのウェイクアップ) */
static int thread_wakeup(kz_thread_id_t id)
{
  puts("thread_wakeup\n");
  putcurrent();

  current = (kz_thread*)id;
  putcurrent();

  return 0;
}

/* システムコールの処理(kz_getid():スレッドID取得) */
static kz_thread_id_t thread_getid(void)
{
  puts("thread_getid\n");
  putcurrent();
  return (kz_thread_id_t)current;
}

/* システムコールの処理(kz_chpri():スレッドの優先度変更) */
static int thread_chpri(int priority)
{
  puts("thread_chpri\n");
  int old = current->priority;
  if(priority >= 0)
    current->priority = priority; /* 優先度変更 */
  putcurrent();
  return old;
}

// /* システムコールの処理(kz_kmalloc():動的メモリ獲得) */
// static void *thread_kmalloc(int size)
// {
//   puts("thread_kmalloc\n");
//   putcurrent();
//   return kzmem_alloc(size);
// }

/* システムコールの処理(kz_ksbrk():動的メモリ獲得) */
static void *thread_ksbrk(int size)
{
  puts("thread_ksbrk\n");
  putcurrent();
  return kzmem_alloc(size);
}

/* 割込みハンドラの登録 */
static int setintr(softvec_type_t type, kz_handler_t handler)
{
  static void thread_intr(softvec_type_t type, unsigned long sp);

  /**
   * 割込みを受け付けるために、ソフトウェア割り込みベクタに
   * OSの割り込み処理の入り口となる関数を登録する
  */
  softvec_setintr(type, thread_intr);

  handlers[type] = handler;

  return 0;
}

static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  puts("call_functions\n");
  /* システムコールの実行中にcurrentが書き変わるので注意 */
  switch(type) {
    case KZ_SYSCALL_TYPE_RUN: /* kz_run() */
      p->un.run.ret = thread_run(p->un.run.func, p->un.run.name,
                                 p->un.run.priority, p->un.run.stacksize,
                                 p->un.run.argc, p->un.run.argv);
      break;
    case KZ_SYSCALL_TYPE_EXIT:
      /* TCBが消去されるので、戻り値を書き込んではいけない */
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

/* システムコールの処理 */
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  puts("syscall_proc\n");
  getcurrent();
  call_functions(type, p);
}

/* スレッドのスケジューリング */
static void schedule(void)
{
  puts("schedule ");
  int i;

  /**
   * 優先度の高い順(優先度の数値の小さい順)にレディーキューを見て，
   * 動作可能なスレッドを検索する．
  */
  for(i = 0; i < PRIORITY_NUM; i++) {
    if(readyque[i].head) /* 見つかった */
      break;
  }
  if(i == PRIORITY_NUM) /* 見つからなかった */
    kz_sysdown();

  current = readyque[i].head; /* カレントスレッドに設定する */
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
  getcurrent(); /* レディーキューから外す */
  thread_exit();
}

/* 割り込み処理の入口関数 */
static void thread_intr(softvec_type_t type, unsigned long sp)
{
  puts("thread_intr\n");
  /* カレントスレッドのコンテキストを保存する */
  current->context.sp = sp;

  /**
   * 割り込みごとの処理を実行する
   * SOFTVEC_TYPE_SYSCALL, SOFTVEC_TYPE_SOFTERRの場合は
   * syscall_intr(), softerr_intr()がハンドラに登録されているので，
   * それらが実行される．
  */
  if(handlers[type])
    handlers[type]();

  schedule();

  puts("dispatch\n");
  dispatch(&current->context);
  /* ここには返ってこない */
}

/* 割り込み処理の入口関数 */
static void timer_intr(softvec_type_t type, unsigned long sp) {
  puts("timer_intr: current->cpu_time: ");
  putxval(current->cpu_time, 3);
  puts("\n");

  /* カレントスレッドのコンテキストを保存する */
  current->context.sp = sp;
  
  /* タイマ継続処理 */
  timer8_clear_flag(0);
  
  /* 1秒ごとの処理 */
  if(++clock_time >= TIMER_HZ) {
    time++; /* 時刻を更新 */

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

  // タイムスライス処理
  current->cpu_time++;
  if(current->cpu_time >= (uint8)(TIMER_HZ * (CPU_QUANTUM_MS / 1000))) {

    getcurrent();
    putcurrent();

    // print_readyque(3);

    current->cpu_time = 0;

    schedule();
    puts("dispatch\n");
    dispatch(&current->context);
    /* ここには返ってこない */
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

  /* 割込みハンドラの登録 */
  setintr(SOFTVEC_TYPE_SYSCALL, syscall_intr);
  setintr(SOFTVEC_TYPE_SOFTERR, softerr_intr);

  softvec_setintr(SOFTVEC_TYPE_TIMINTR, timer_intr);
  timer8_hz(0);
  
  /* システムコール発行不可なので直接関数を呼び出して動的メモリ獲得 */
  thread_ksbrk(INIT_MEMORY_SIZE);

  /* システムコール発行不可なので直接関数を呼び出してスレッドを作成する */
  current = (kz_thread*)thread_run(func, name, priority, stacksize, argc, argv);


  /* 最初のスレッドを起動 */
  puts("dispatch\n");
  dispatch(&current->context);

  /* ここには返ってこない */
}

void kz_sysdown(void)
{
  puts("system error!\n");
  timer8_stop(0);
  while(1)
    ;
}

/* システムコール呼び出し用ライブラリ関数 */
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
  puts("kz_syscall\n");
  current->syscall.type  = type;
  current->syscall.param = param;
  asm volatile ("trapa #0"); /* トラップ割込み発行(intr_syscall) */
}