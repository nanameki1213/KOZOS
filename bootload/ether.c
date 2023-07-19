#include "defines.h"
#include "ether.h"
#include "timer.h"

#define PACKET_MAX_NUM 256

static unsigned char src_MAC[6];
static unsigned char src_IP[4];
static unsigned char dst_MAP[6];
static unsigned char dst_IP[4];
static unsigned char packet[PACKET_MAX_NUM];

#define H8_3069F_IO_NUM 9

#define H8_3069F_IO_P1DR 0x0fffd0
#define H8_3069F_IO_P2DR 0x0fffd1
#define H8_3069F_IO_P3DR 0x0fffd2
#define H8_3069F_IO_P4DR 0x0fffd3
#define H8_3069F_IO_P5DR 0x0fffd4
#define H8_3069F_IO_P6DR 0x0fffd5
#define H8_3069F_IO_P7DR 0x0fffd6
#define H8_3069F_IO_P8DR 0x0fffd7
#define H8_3069F_IO_P9DR 0x0fffd8

#define H8_3069F_IO_P1DDR 0x0ee000
#define H8_3069F_IO_P2DDR 0x0ee001
#define H8_3069F_IO_P3DDR 0x0ee002
#define H8_3069F_IO_P4DDR 0x0ee003
#define H8_3069F_IO_P5DDR 0x0ee004
#define H8_3069F_IO_P6DDR 0x0ee005
#define H8_3069F_IO_P7DDR 0x0ee006
#define H8_3069F_IO_P8DDR 0x0ee007
#define H8_3069F_IO_P9DDR 0x0ee008

#define RTL8019_BASE 0x200000

#define RTL8019_IORB 

#define RTL8019_CR RTL8019_BASE
#define RTL8019_RDMAP 0x2000010
#define RTL8019_RP    0x2000018

#define H8_3069F_P64DR (1<<4)
#define H8_3069F_P65DR (1<<5)

static uint8 io_pdr[H8_3069F_IO_NUM] = {
  H8_3069F_IO_P1DR,
  H8_3069F_IO_P2DR,
  H8_3069F_IO_P3DR,
  H8_3069F_IO_P4DR,
  H8_3069F_IO_P5DR,
  H8_3069F_IO_P6DR,
  H8_3069F_IO_P7DR,
  H8_3069F_IO_P8DR,
  H8_3069F_IO_P9DR,
};

static uint8 io_pddr[H8_3069F_IO_NUM] = {
  H8_3069F_IO_P1DDR,
  H8_3069F_IO_P2DDR,
  H8_3069F_IO_P3DDR,
  H8_3069F_IO_P4DDR,
  H8_3069F_IO_P5DDR,
  H8_3069F_IO_P6DDR,
  H8_3069F_IO_P7DDR,
  H8_3069F_IO_P8DDR,
  H8_3069F_IO_P9DDR,
};

/* ページ0のレジスタ */
#define RTL8019_PSTART (RTL8019_BASE + 0x1)
#define RTL8019_PSTOP  (RTL8019_BASE + 0x2)
#define RTL8019_BNRY   (RTL8019_BASE + 0x3)
#define RTL8019_TPSR   (RTL8019_BASE + 0x4)
#define RTL8019_TBCR0  (RTL8019_BASE + 0x5)
#define RTL8019_TBCR1  (RTL8019_BASE + 0x6)
#define RTL8019_ISR    (RTL8019_BASE + 0x7)
#define RTL8019_RSAR0  (RTL8019_BASE + 0x8)
#define RTL8019_RSAR1  (RTL8019_BASE + 0x9)
#define RTL8019_RBCR0  (RTL8019_BASE + 0xa)
#define RTL8019_RBCR1  (RTL8019_BASE + 0xb)
#define RTL8019_RCR    (RTL8019_BASE + 0xc)
#define RTL8019_TCR    (RTL8019_BASE + 0xd)
#define RTL8019_DCR    (RTL8019_BASE + 0xe)
#define RTL8019_IMR    (RTL8019_BASE + 0xf)

/* ページ1のレジスタ */
#define RTL8019_PAR0 (RTL8019_BASE + 0x1)
#define RTL8019_PAR1 (RTL8019_BASE + 0x2)
#define RTL8019_PAR2 (RTL8019_BASE + 0x3)
#define RTL8019_PAR3 (RTL8019_BASE + 0x4)
#define RTL8019_PAR4 (RTL8019_BASE + 0x5)
#define RTL8019_PAR5 (RTL8019_BASE + 0x6)
#define RTL8019_CURR (RTL8019_BASE + 0x7)
#define RTL8019_MAR0 (RTL8019_BASE + 0x8)
#define RTL8019_MAR1 (RTL8019_BASE + 0x9)
#define RTL8019_MAR2 (RTL8019_BASE + 0xa)
#define RTL8019_MAR3 (RTL8019_BASE + 0xb)
#define RTL8019_MAR4 (RTL8019_BASE + 0xc)
#define RTL8019_MAR5 (RTL8019_BASE + 0xd)
#define RTL8019_MAR6 (RTL8019_BASE + 0xe)
#define RTL8019_MAR7 (RTL8019_BASE + 0xf)

static 

unsigned char rtl8019_read(unsigned char addr)
{
  unsigned char data;

  io_pdr[0] = addr; /* ポート1二レジスタのアドレスをセットする */  
  io_pddr[2] = 0;
  io_pdr[5] &= ~H8_3069F_P64DR;
  data = io_pdr[2];
  io_pdr[5] |= H8_3069F_P64DR;
  return data;
}

void rtl8019_write(unsigned char addr, unsigned char data)
{
  io_pdr[0] = addr;
  io_pddr[2] = 0xff;
  io_pdr[5] &= ~H8_3069F_P65DR;
  io_pdr[2] = data;
  io_pdr[5] |= H8_3069F_P65DR;
}

void rtl8019_init(void)
{
  uint16 i;
  uint8 data;


  data = rtl8019_read(RTL8019_RP);
  rtl8019_write(RTL8019_RP, data);
  timer8_start(0, 10);

  rtl8019_write(RTL8019_CR, 0x21);
  rtl8019_write(RTL8019_DCR, 0x4a);
  rtl8019_write(RTL8019_RBCR0, 0);
  rtl8019_write(RTL8019_RBCR1, 0);
  rtl8019_write(RTL8019_RCR, 0x20);
  rtl8019_write(RTL8019_TCR, 0x02);
  rtl8019_write(RTL8019_TPSR, 0x40);
  rtl8019_write(RTL8019_PSTART, 0x46);
  rtl8019_write(RTL8019_PSTOP, 0x60);
  rtl8019_write(RTL8019_IMR, 0x00);
  rtl8019_write(RTL8019_ISR, 0xff);

  rtl8019_write(RTL8019_RBCR0, 12);
  rtl8019_write(RTL8019_RBCR1, 0);
  rtl8019_write(RTL8019_RSAR0, 0x00);
  rtl8019_write(RTL8019_RSAR1, 0x00);
  rtl8019_write(RTL8019_CR, 0x0a);
  for(i = 0; i < 6; i += 2) {
    src_MAC[i+1] = (RTL8019_RDMAP);
    rtl8019_read(RTL8019_RDMAP);
    src_MAC[i] = rtl8019_read(RTL8019_RDMAP);
    rtl8019_read(RTL8019_RDMAP);
  }
  do {
    data = rtl8019_read(RTL8019_ISR);
  } while((data & 0x40) == 0x00); /* リモートDMAが停止するのを待つ */

  rtl8019_write(RTL8019_CR, 0x61);
  rtl8019_write(RTL8019_PAR0, src_MAC[0]);
  rtl8019_write(RTL8019_PAR1, src_MAC[1]);
  rtl8019_write(RTL8019_PAR2, src_MAC[2]);
  rtl8019_write(RTL8019_PAR3, src_MAC[3]);
  rtl8019_write(RTL8019_PAR4, src_MAC[4]);
  rtl8019_write(RTL8019_PAR5, src_MAC[5]);

  rtl8019_write(RTL8019_CURR, 0x47);

  rtl8019_write(RTL8019_MAR0, 0);
  rtl8019_write(RTL8019_MAR1, 0);
  rtl8019_write(RTL8019_MAR2, 0);
  rtl8019_write(RTL8019_MAR3, 0);
  rtl8019_write(RTL8019_MAR4, 0);
  rtl8019_write(RTL8019_MAR5, 0);
  rtl8019_write(RTL8019_MAR6, 0);
  rtl8019_write(RTL8019_MAR7, 0);

  rtl8019_write(RTL8019_CR, 0x21);
  rtl8019_write(RTL8019_RCR, 0x04);
  rtl8019_write(RTL8019_TCR, 0x00);
  rtl8019_write(RTL8019_IMR, 0x00);
}