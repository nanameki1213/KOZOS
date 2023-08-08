#ifndef _KOZOS_MEMORY_H_INCLUDED_
#define _KOZOS_MEMORY_H_INCLUDED_

void *kmalloc(int size);
void kfree(void *mem);
void *kzmem_alloc(int size);
void kzmem_free(void *mem);

#endif