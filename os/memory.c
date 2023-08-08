#include "defines.h"
#include "kozos.h"
#include "lib.h"
#include "memory.h"
#include "interrupt.h"

#define NALLOC 256

typedef struct _kzmem_block {
  struct _kzmem_block *next;
  int size;
} kzmem_block;

static kzmem_block *freep = NULL;

static char *area;


/**
 * @brief ?????????????????????
 * 
 * @param size ????????????????????????????
 * @return void* ?????????????????????????????????????
 */
void *kmalloc(int size)
{
  INTR_DISABLE;

  int i;
  kzmem_block *p, *prevp;
  unsigned nunits;
  extern char freearea;

  nunits = (size + sizeof(kzmem_block));
  if(freep == NULL) {
    area = &freearea;
    freep = (kzmem_block*)area;
    freep->size = sizeof(kzmem_block);
    freep->next = (kzmem_block*)area;
  }
  prevp = freep;
  if(prevp != freep)
    puts("why!?\n");
  for(p = prevp->next; ; prevp = p, p = p->next) {
    if(p->size >= nunits) {
      if(p->size == nunits) {
        prevp->next = p->next;
      }
      else {
        p->size -= nunits;
        p += p->size;
        p->size = nunits;
      }
      freep = prevp;
      INTR_ENABLE;
      return (void*)(p + 1);
    }
    if(p == freep) {
      puts("hello\n");
      INTR_ENABLE;
      if((p = kz_sbrk(nunits)) == NULL) {
        kz_sysdown();
      }
      INTR_DISABLE;
    }
  }

}

void kfree(void *mem)
{
  INTR_DISABLE;
  kzmem_block *bp, *p;

  bp = (kzmem_block*)mem - 1;
  for(p = freep; !(bp > p && bp < p->next); p = p->next) {
    if(p >= p->next && (bp > p || bp < p->next))
      break; /* 領域の始めあるいは終わりの解放ブロック */
  }

  if(bp + bp->size == p->next) {
    bp->size += p->next->size;
    bp->next = p->next->next;
  } else {
    bp->next = p->next;
  }
  if(p + p->size == bp) {
    p->size += bp->size;
    p->next = bp->next;
  } else {
    p->next = bp;
  }
  freep = p;
  INTR_ENABLE;
}

void *kzmem_alloc(int size)
{
  char *cp;
  kzmem_block *up;

  cp = area;
  up = (kzmem_block*)cp;

  if(size < NALLOC)
    size = NALLOC;
  area += size; // メモリ確保
  up->size = size;
  return (void*)(up + 1);
}

/**
 * @brief ??????????????
 * 
 * @param mem kzmem_alloc?????????????????????????????????????????????? 
 */
// void kzmem_free(void *mem)
// {
//   int i;
//   kzmem_block *mp;
//   kzmem_pool *p;

//   /* ????????????????????????????????????????????????????????????????? */
//   mp = ((kzmem_block*)mem - 1);

//   for(i = 0; i < MEMORY_AREA_NUM; i++) {
//     p = &freep[i];
//     if(mp->size == p->size) {
//       /* ????????????????????????????????????????? */
//       mp->next = p->free;
//       p->free = mp;
//       return;
//     }
//   }

//   kz_sysdown();
// }