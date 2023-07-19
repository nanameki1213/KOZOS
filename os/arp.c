#include "ether.h"
#include "defines.h"
#include "kozos.h"

int arp_main(int argc, char *argv[])
{
  puts("arp_main started.\n");
  unsigned short i;
   unsigned char packet_type;
   uint8 str_IP[16];
   uint8 str_MAC[13];
   ARP_PACKET *arp_packet;
   
   io_regs_init();
   puts("reg init\n");
   rtl8019_init(); /* RTL8019ASの初期化 */
   puts("lan controller init\n");
   
   /* 送信元IPアドレスの設定 */
   src_IP[0] = 192;
   src_IP[1] = 168;
   src_IP[2] = 11;
   src_IP[3] = 100;
   
   /* 宛先IPアドレスの設定 */
   dst_IP[0] = 192;
   dst_IP[1] = 168;
   dst_IP[2] = 11;
   dst_IP[3] = 17;
   
  //  kz_sleep(2000); /* 2000ms（2秒）の時間待ち */
   
   ARP_request(packet); /* ARPリクエスト */
   
   packet_type = 0; /* packet_typeを0にする */
   do
   {
      if (packet_receive(packet) != 1) /* パケットを受信したとき */
      {
         arp_packet = (ARP_PACKET *)packet; /* packetをARP_PACKET構造体に当てはめる */
         if ((arp_packet -> eth_ethernet_type == 0x0806) && 
             /* ARPのパケットのとき */
             (IP_compare(arp_packet -> arp_dst_IP, src_IP) == 0) && 
             /* パケットに記述されている宛先IPアドレスが送信元IPアドレスに一致したとき */
             (arp_packet -> arp_operation == 2)
             /* ARPリプライのとき */
            )
         {
            for (i = 0; i < 6; i++)
            {
               dst_MAC[i] = arp_packet -> arp_src_MAC[i];
               /* パケットに記述されている送信元MACアドレスをdst_MACに格納する */
            }
            packet_type = 'a'; /* packet_typeを'a'にする */
         }
      }
   } while (packet_type != 'a'); /* packet_typeが'a'になるまで待つ */
   
   MAC_to_str(dst_MAC, str_MAC); /* dst_MACをstr_MAC（文字列）に変換する */
   puts(str_MAC);
   puts("\narp end.\n");
}