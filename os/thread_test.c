#include "defines.h"
#include "kozos.h"
#include "lib.h"
#include "memory.h"

int test_1_main(int argc, char *argv[])
{
  char *buf = kmalloc(128);
  kfree(buf);
  return 0;
}

int test_2_main(int argc, char *argv[])
{
  char *buf = kmalloc(8);
  kfree(buf);

  return 0;
}

int test_3_main(int argc, char *argv[])
{
  char *buf = kmalloc(64);
  kfree(buf);

  return 0;
}