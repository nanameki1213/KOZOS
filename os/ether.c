#include "defines.h"
#include "ether.h"
#include "kozos.h"

#define PACKET_MAX_NUM 256
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
  kz_sleep(10);

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
  rtl8019_write(RTL8019_CR, 0x22);
  rtl8019_write(RTL8019_TCR, 0x00);
  rtl8019_write(RTL8019_IMR, 0x00);
}

void packet_send(unsigned char *packet, unsigned short size)
{
  uint16 i;
  uint8 data;
  uint8 size_H, size_L;

  size_L = (uint8)(size &0x00ff);
  size_H = (uint8)(size >>8);

  rtl8019_write(RTL8019_CR, 0x22);
  rtl8019_write(RTL8019_RBCR0, size_L);
  rtl8019_write(RTL8019_RBCR1, size_H);
  rtl8019_write(RTL8019_RSAR0, 0x00);
  rtl8019_write(RTL8019_RSAR1, 0x40);
  rtl8019_write(RTL8019_CR, 0x12);
  for(i = 0; i < size; i++) {
    rtl8019_write(RTL8019_RDMAP, packet[i]);
  } 
  do {
    data = rtl8019_read(RTL8019_ISR);
  } while((data & 0x40) == 0x00);

  rtl8019_write(RTL8019_CR, 0x22);
  rtl8019_write(RTL8019_TBCR0, size_L);
  rtl8019_write(RTL8019_TBCR1, size_H);
  rtl8019_write(RTL8019_TPSR, 0x40);
  rtl8019_write(RTL8019_CR, 0x26);
  do {
    data = rtl8019_read(RTL8019_CR);
  } while((data & 0x04) == 0x04);
}

unsigned char packet_receive(unsigned char *packet)
{
  uint16 i;
  uint16 size;
  uint8 data;
  uint8 size_H, size_L;
  uint8 boundary_page, start_page, current_page;
  uint8 header[4];

  rtl8019_write(RTL8019_CR, 0x22);
  boundary_page = rtl8019_read(RTL8019_BNRY);
  rtl8019_write(RTL8019_CR, 0x62);
  current_page = rtl8019_read(RTL8019_CURR);

  if(current_page < boundary_page) {
    current_page += (0x60 - 0x46);
  }
  if(current_page == boundary_page + 1) {
    return 1;
  }

  start_page = boundary_page + 1;
  if(start_page = 0x60) {
    start_page = 0x46;
  }

  rtl8019_write(RTL8019_CR, 0x22); /* レジスタ・ページ0 */
   rtl8019_write(RTL8019_RBCR0, 4); /* フレーム管理ヘッダのサイズ（下位バイト） */
   rtl8019_write(RTL8019_RBCR1, 0); /* フレーム管理ヘッダのサイズ（上位バイト） */
   rtl8019_write(RTL8019_RSAR0, 0x00); /* フレーム管理ヘッダの先頭アドレス（下位バイト） */
   rtl8019_write(RTL8019_RSAR1, start_page); /* フレーム管理ヘッダの先頭アドレス（上位バイト） */
   rtl8019_write(RTL8019_CR, 0x0A); /* リモートDMA読み取りを許可する */
   for (i = 0; i < 4; i++)
   {
      header[i] = rtl8019_read(RTL8019_RDMAP); /* フレーム管理ヘッダを読み取る */
   }
   do
   {
      data = rtl8019_read(RTL8019_ISR); /* ISRを読み取る */
   } while ((data & 0x40) == 0x00); /* リモートDMAが停止するのを待つ */
   
   rtl8019_write(RTL8019_CR, 0x22); /* レジスタ・ページ0 */
   size_L = header[2]; /* 受信パケットのサイズ（下位バイト） */
   size_H = header[3]; /* 受信パケットのサイズ（上位バイト） */
   size = ((unsigned short)size_H << 8) + (unsigned short)size_L; /* 受信パケットのサイズ */
   rtl8019_write(RTL8019_RBCR0, size_L); /* 受信パケットのサイズ（下位バイト） */
   rtl8019_write(RTL8019_RBCR1, size_H); /* 受信パケットのサイズ（上位バイト） */
   rtl8019_write(RTL8019_RSAR0, 0x00); /* 受信パケットの先頭アドレス（下位バイト） */
   rtl8019_write(RTL8019_RSAR1, start_page); /* 受信パケットの先頭アドレス（上位バイト） */
   rtl8019_write(RTL8019_CR, 0x0A); /* リモートDMA読み取りを許可する */
   for (i = 0; i < 4; i++)
   {
      rtl8019_read(RTL8019_RDMAP); /* ダミー・データを読み取る */
   }
   for (i = 0; i < (size - 4); i++)
   {
      packet[i] = rtl8019_read(RTL8019_RDMAP); /* 受信バッファからパケットのデータを読み取る */
      
      if (i >= 256) /* パケットのサイズが256byteよりも大きいとき */
      {
         rtl8019_read(RTL8019_RDMAP); /* ダミー・データを読み取る */
      }
   }
   do
   {
      data = rtl8019_read(RTL8019_ISR); /* ISRを読み取る */
   } while ((data & 0x40) == 0x00); /* リモートDMAが停止するのを待つ */
   
   rtl8019_write(RTL8019_CR, 0x22); /* レジスタ・ページ0 */
   boundary_page = current_page - 1;
   if (boundary_page >= 0x60)
   {
      boundary_page -= (0x60 - 0x46); /* 受信バッファがリング・バッファであることを考慮する */
   }
   rtl8019_write(RTL8019_BNRY, boundary_page); /* BNRYを更新する */
   
   return 0;
}

void ARP_request(unsigned char *packet)
{
   unsigned short i;
   ARP_PACKET *arp_packet;
   
   arp_packet = (ARP_PACKET *)packet; /* packetをARP_PACKET構造体に当てはめる */
   
   for (i = 0; i < 6; i++)
   {
      arp_packet -> eth_dst_MAC[i] = 0xFF; /* ブロードキャスト・アドレス */
   }
   for (i = 0; i < 6; i++)
   {
      arp_packet -> eth_src_MAC[i] = src_MAC[i]; /* 送信元MACアドレス */
   }
   arp_packet -> eth_ethernet_type = 0x0806; /* 上位プロトコルの種類（ARP） */
   arp_packet -> arp_hardware_type = 0x0001; /* ネットワークの物理媒体の種類（イーサネット） */
   arp_packet -> arp_protocol_type = 0x0800; /* 上位プロトコルの種類（IP） */
   arp_packet -> arp_hardware_length = 6; /* ネットワークの物理媒体のアドレス長（MACアドレス） */
   arp_packet -> arp_protocol_length = 4; /* 上位プロトコルのアドレス長（IPv4） */
   arp_packet -> arp_operation = 1; /* ARPの動作（ARPリクエスト） */
   for (i = 0; i < 6; i++)
   {
      arp_packet -> arp_src_MAC[i] = src_MAC[i]; /* 送信元MACアドレス */
   }
   for (i = 0; i < 4; i++)
   {
      arp_packet -> arp_src_IP[i] = src_IP[i]; /* 送信元IPアドレス */
   }
   for (i = 0; i < 6; i++)
   {
      arp_packet -> arp_dst_MAC[i] = 0x00; /* 宛先MACアドレス */
   }
   for (i = 0; i < 4; i++)
   {
      arp_packet -> arp_dst_IP[i] = dst_IP[i]; /* 宛先IPアドレス */
   }
   
   packet_send(packet, 60); /* パケットのサイズを60byteに指定して送信する */
}

void ARP_reply(unsigned char *packet)
{
   unsigned short i;
   ARP_PACKET *arp_packet;
   
   arp_packet = (ARP_PACKET *)packet; /* packetをARP_PACKET構造体に当てはめる */
   
   for (i = 0; i < 6; i++)
   {
      arp_packet -> eth_dst_MAC[i] = arp_packet -> eth_src_MAC[i];
      arp_packet -> eth_src_MAC[i] = src_MAC[i];
      /* 宛先MACアドレスと送信元MACアドレスを入れ替える */
   }
   arp_packet -> arp_operation = 2; /* ARPの動作（ARPリプライ） */
   for (i = 0; i < 6; i++)
   {
      arp_packet -> arp_dst_MAC[i] = arp_packet -> arp_src_MAC[i];
      arp_packet -> arp_src_MAC[i] = src_MAC[i];
      /* 宛先MACアドレスと送信元MACアドレスを入れ替える */
   }
   for (i = 0; i < 4; i++)
   {
      arp_packet -> arp_dst_IP[i] = arp_packet -> arp_src_IP[i];
      arp_packet -> arp_src_IP[i] = src_IP[i];
      /* 宛先IPアドレスと送信元IPアドレスを入れ替える */
   }
   
   packet_send(packet, 60); /* パケットのサイズを60byteに指定して送信する */
}

void IP_to_str(unsigned char *IP, char *str)
{
   unsigned short i, n;
   
   n = 0;
   for (i = 0; i < 4; i++)
   {
      str[n] = '0' + (IP[i] / 100);
      /* '0'の文字コードを基準として，IP[i]を10進数で表したときの100の位を文字に変換する */
      
      n++;
      str[n] = '0' + ((IP[i] % 100) / 10);
      /* '0'の文字コードを基準として，IP[i]を10進数で表したときの10の位を文字に変換する */
      
      n++;
      str[n] = '0' + (IP[i] % 10);
      /* '0'の文字コードを基準として，IP[i]を10進数で表したときの1の位を文字に変換する */
      
      if (i < 3)
      {
         n++;
         str[n] = '.'; /* 区切り文字（ピリオド） */
      }
      
      n++;
   }
   str[n] = 0x00; /* 文字列の最後をナル文字にする */
}

void MAC_to_str(unsigned char *MAC, char *str)
{
   unsigned short i, n;
   unsigned char nibble;
   
   n = 0;
   for (i = 0; i < 6; i++)
   {
      nibble = (MAC[i] & 0xF0) >> 4; /* MAC[i]の上位4bit */
      if (nibble < 10) /* nibbleが10よりも小さいとき */
      {
         str[n] = '0' + nibble;
         /* '0'の文字コードを基準として，nibbleを文字に変換する */
      }
      else /* MAC[i]の上位4bitが10以上のとき */
      {
         str[n] = 'A' + nibble - 10;
         /* 'A'の文字コードを基準として，nibbleを文字に変換する */
      }
      
      n++;
      nibble = MAC[i] & 0x0F; /* MAC[i]の下位4bit */
      if (nibble < 10) /* nibbleが10よりも小さいとき */
      {
         str[n] = '0' + nibble;
         /* '0'の文字コードを基準として，nibbleを文字に変換する */
      }
      else /* MAC[i]の下位4bitが10以上のとき */
      {
         str[n] = 'A' + nibble - 10;
         /* 'A'の文字コードを基準として，nibbleを文字に変換する */
      }
      
      n++;
   }
   str[n] = 0x00; /* 文字列の最後をナル文字にする */
}

unsigned char str_to_IP(char *str, unsigned char *IP)
{
   unsigned short i, n;
   unsigned short sum;
   
   n = 0;
   
   for (i = 0; i < 4; i++)
   {
      sum = 0;
      while (n < 15)
      {
         sum *= 10;
         sum += (str[n] - '0'); /* '0'の文字コードを基準として，文字を数字に変換する */
         n++;
         if ((str[n] == '.') || (str[n] == ' ')) /* 区切り文字（ピリオドまたは空白） */
         {
            n++;
            break;
         }
      }
      if (sum < 256) /* エラー・チェック */
      {
         IP[i] = (unsigned char)sum; /* エラーがなければ，sumをIP[i]に代入する */
      }
      else
      {
         return 1; /* エラーがあれば，1を戻り値として，呼び出し元の関数に戻る */
      }
   }
   
   return 0; /* エラーがなければ，0を戻り値として，呼び出し元の関数に戻る */
}

unsigned char IP_compare(unsigned char *IP_a, unsigned char *IP_b)
{
   unsigned short i;
   
   for (i = 0; i < 4; i++)
   {
      if (IP_a[i] != IP_b[i])
      {
         return 1; /* IPアドレスが一致しなければ，1を戻り値として，呼び出し元の関数に戻る */
      }
   }
   
   return 0; /* IPアドレスが一致すれば，0を戻り値として，呼び出し元の関数に戻る */
}

unsigned long ones_complement_sum(unsigned char *data, unsigned short offset, unsigned short size)
{
   unsigned short i;
   unsigned long sum;
   
   sum = 0;
   for (i = offset; i < (offset + size); i += 2)
   {
      sum += ((unsigned long)data[i] << 8) + (unsigned long)data[i + 1];
      /* data[i]を上位バイト，data[i + 1]を下位バイトとして，2byte単位で合計値を計算する */
      
      sum = (sum & 0xFFFF) + (sum >> 16);
      /* sumが0xFFFFよりも大きいときは，最上位の桁を消去し，あらためて1を加算する */
   }
   
   return sum; /* sumを戻り値として，呼び出し元の関数に戻る */
}

void io_regs_init(void)
{
   /* RTL8019ASのアドレス・バス */
   
   io_pddr[0] = 0xFF; /* P10~P17を出力端子として設定する */
   io_pdr[0] = 0xFF; /* P10~P17の初期値を1に設定する */
   
   /* ディップ・スイッチ */
   io_pddr[1] = 0x00; /* P20~P27を入力端子として設定する */
   io_pdr[1] = 0xFF; /* P20~P27のプルアップ回路を有効にする */
   
   /* RTL8019ASのデータ・バス */
   io_pddr[2] = 0xFF; /* P30~P37を出力端子として設定する */
   io_pdr[2] = 0xFF; /* P30~P37の初期値を1に設定する */
   
   /* マトリクス・キー */
   io_pddr[3] = 0xF0; /* P40~P43を入力端子，P44~P47を出力端子として設定する */
   io_pdr[3] = 0x0F; /* P40~P43のプルアップ回路を有効にする */
   
   /* ONスイッチ，OFFスイッチ */
   io_pddr[4] = 0xF0; /* P50~P53を入力端子として設定する */
   io_pdr[4] = 0xFF; /* P50~P53のプルアップ回路を有効にする */
   
   /* RTL8019ASの制御バス */
   io_pddr[5] = 0xFF; /* P60~P67を出力端子として設定する */
   io_pdr[5] = 0xFF; /* P60~P67の初期値を1に設定する */
}
