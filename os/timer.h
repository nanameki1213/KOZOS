#ifndef _TIMER_H_INCLUDED_
#define _TIMER_H_INCLUDED_

#define TIMER_8_NUM 2
#define TIMER_16_NUM 3

#define H8_3069F_TIMER8_INTERNAL8 1
#define H8_3069F_TIMER8_INTERNAL64 2
#define H8_3069F_TIMER8_INTERNAL8192 3
#define H8_3069F_TIMER8_VARIETY 4
#define H8_3069F_TIMER8_EXTERNAL_UP 5
#define H8_3069F_TIMER8_EXTERNAL_DOWN 6
#define H8_3069F_TIMER8_EXTERNAL_BOTH 7

// int timer8_start(int index, int msec);
int timer8_hz(int index);
int timer8_stop(int index);
int timer8_clear_flag(int index);

#endif
