#include "lib.h"
#include "defines.h"
#include "timer.h"

/* とりあえず8ビットタイマだけ */

struct h8_3069f_timer8 {
  volatile uint8 tcr0;
  volatile uint8 tcr1;
  volatile uint8 tcsr0;
  volatile uint8 tcsr1;
  volatile uint8 tcora0;
  volatile uint8 tcora1;
  volatile uint8 tcorb0;
  volatile uint8 tcorb1;
  volatile uint16 tcnt;
};

#define H8_3069F_TIMER8_01 ((volatile struct h8_3069f_timer8 *)0xffff80)
#define H8_3069F_TIMER8_23 ((volatile struct h8_3069f_timer8 *)0xffff90)

#define H8_3069F_TIMER8_TCR_CKS0  (1<<0)
#define H8_3069F_TIMER8_TCR_CKS1  (1<<1)
#define H8_3069F_TIMER8_TCR_CKS2  (1<<2)
#define H8_3069F_TIMER8_TCR_CCLR0 (1<<3)
#define H8_3069F_TIMER8_TCR_CCLR1 (1<<4)
#define H8_3069F_TIMER8_TCR_OVIE  (1<<5)
#define H8_3069F_TIMER8_TCR_CMIEA (1<<6)
#define H8_3069F_TIMER8_TCR_CMIEB (1<<7)

#define H8_3069F_TIMER8_TCSR_CMIEA (1<<6)

/* TCSRレジスタ */
#define H8_3069F_TIMER8_TCSR_ICE (1<<4)

/* マクロ */ 
#define H8_3069F_TIMER8_CLOCK_DISABLE (uint8)248U
#define H8_3069F_TIMER8_CLEAR_DISABLE (uint8)~(3<<3)

#define H8_3069F_TIMER16_0 ((volatile struct h8_3069f_timer16 *)0xffff68)
#define H8_3069F_TIMER16_1 ((volatile struct h8_3069f_timer16 *)0xffff70)
#define H8_3069F_TIMER16_2 ((volatile struct h8_3069f_timer16 *)0xffff78)

static struct {
  volatile struct h8_3069f_timer8 *tiemr8;
} timer8_regs[TIMER_8_NUM] = {
  { H8_3069F_TIMER8_01 },
  { H8_3069F_TIMER8_23 },
};

// int timer8_start(int index, int msec) {
//   volatile struct h8_3069f_timer8 *timer8 = timer8_regs[index].tiemr8;
//   int count;

//   timer8->tcr0 = H8_3069F_TIMER8_VARIETY | H8_3069F_TIMER8_TCR_CCLR0;
//   timer8->tcr1 = H8_3069F_TIMER8_INTERNAL8192;

//   count = msec / 105;

//   timer8->tcnt = 0;
//   timer8->tcora0 = count;
//   timer8->tcr0 |= H8_3069F_TIMER8_TCR_CMIEA;

//   return 0;
// }

int timer8_hz(int index) {
  volatile struct h8_3069f_timer8 *timer8 = timer8_regs[index].tiemr8;

  timer8->tcr0 = H8_3069F_TIMER8_INTERNAL8192 | H8_3069F_TIMER8_TCR_CCLR0;

  timer8->tcnt = 0;
  timer8->tcora0 = 2441 / TIMER_HZ;

  timer8->tcr0 |= H8_3069F_TIMER8_TCR_CMIEA;

  return 0;
}

int timer8_stop(int index) {
  volatile struct h8_3069f_timer8 *timer8 = timer8_regs[index].tiemr8;

  timer8->tcr0 &= (uint8)~H8_3069F_TIMER8_TCR_CMIEA;
  
  return 0;
}

int timer8_clear_flag(int index)
{
  volatile struct h8_3069f_timer8 *timer8 = timer8_regs[index].tiemr8;

  timer8->tcr0 &= ~H8_3069F_TIMER8_TCR_CMIEA;

  timer8->tcnt = 0;
  timer8->tcsr0 &= ~H8_3069F_TIMER8_TCR_CMIEA;

  timer8->tcr0 |= H8_3069F_TIMER8_TCSR_CMIEA;

  return 0;
  
}