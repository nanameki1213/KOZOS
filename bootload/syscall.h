#ifndef _KOZOS_SYSCALL_H_INCLUDED_
#define _KOZOS_SYSCALL_H_INCLUDED_

#include "defines.h"

/* �����ƥॳ�����ֹ����� */
typedef enum {
  KZ_SYSCALL_TYPE_RUN = 0,
  KZ_SYSCALL_TYPE_EXIT,
} kz_syscall_type_t;

/* �����ƥॳ����ƤӽФ����Υѥ�᡼����Ǽ������ */
typedef struct {
  union {
    struct {
      kz_func_t
    }
  }
}

#endif