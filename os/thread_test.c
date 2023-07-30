#include "defines.h"
#include "kozos.h"
#include "lib.h"

int test_1_main(int argc, char *argv[])
{
  kz_sleep(1);
  while(1) {
    puts("a");
  }

  return 0;
}

int test_2_main(int argc, char *argv[])
{
  while(1) {
    puts("b");
  }

  return 0;
}

int test_3_main(int argc, char *argv[])
{
  while(1) {
    puts("c");
  }

  return 0;
}