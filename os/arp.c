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
   rtl8019_init(); /* RTL8019AS�ν���� */
   puts("lan controller init\n");
   
   /* ������IP���ɥ쥹������ */
   src_IP[0] = 192;
   src_IP[1] = 168;
   src_IP[2] = 11;
   src_IP[3] = 100;
   
   /* ����IP���ɥ쥹������ */
   dst_IP[0] = 192;
   dst_IP[1] = 168;
   dst_IP[2] = 11;
   dst_IP[3] = 17;
   
  //  kz_sleep(2000); /* 2000ms��2�áˤλ����Ԥ� */
   
   ARP_request(packet); /* ARP�ꥯ������ */
   
   packet_type = 0; /* packet_type��0�ˤ��� */
   do
   {
      if (packet_receive(packet) != 1) /* �ѥ��åȤ���������Ȥ� */
      {
         arp_packet = (ARP_PACKET *)packet; /* packet��ARP_PACKET��¤�Τ����ƤϤ�� */
         if ((arp_packet -> eth_ethernet_type == 0x0806) && 
             /* ARP�Υѥ��åȤΤȤ� */
             (IP_compare(arp_packet -> arp_dst_IP, src_IP) == 0) && 
             /* �ѥ��åȤ˵��Ҥ���Ƥ��밸��IP���ɥ쥹��������IP���ɥ쥹�˰��פ����Ȥ� */
             (arp_packet -> arp_operation == 2)
             /* ARP��ץ饤�ΤȤ� */
            )
         {
            for (i = 0; i < 6; i++)
            {
               dst_MAC[i] = arp_packet -> arp_src_MAC[i];
               /* �ѥ��åȤ˵��Ҥ���Ƥ���������MAC���ɥ쥹��dst_MAC�˳�Ǽ���� */
            }
            packet_type = 'a'; /* packet_type��'a'�ˤ��� */
         }
      }
   } while (packet_type != 'a'); /* packet_type��'a'�ˤʤ�ޤ��Ԥ� */
   
   MAC_to_str(dst_MAC, str_MAC); /* dst_MAC��str_MAC��ʸ����ˤ��Ѵ����� */
   puts(str_MAC);
   puts("\narp end.\n");
}