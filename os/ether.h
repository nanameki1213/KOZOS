#ifndef _ETHER_H_INCLUDED_
#define _ETHER_H_INCLUDED_

#define PACKET_MAX_NUM 256

typedef struct
{
   /* �������ͥåȡ��إå���14byte�� */
   unsigned char eth_dst_MAC[6];
   unsigned char eth_src_MAC[6];
   unsigned short eth_ethernet_type;
   
   /* ARP��å�������28byte�� */
   unsigned short arp_hardware_type;
   unsigned short arp_protocol_type;
   unsigned char arp_hardware_length;
   unsigned char arp_protocol_length;
   unsigned short arp_operation;
   unsigned char arp_src_MAC[6];
   unsigned char arp_src_IP[4];
   unsigned char arp_dst_MAC[6];
   unsigned char arp_dst_IP[4];
} ARP_PACKET;

typedef struct
{
   /* �������ͥåȡ��إå���14byte�� */
   unsigned char eth_dst_MAC[6];
   unsigned char eth_src_MAC[6];
   unsigned short eth_ethernet_type;
   
   /* IP�إå���20byte�� */
   unsigned char ip_version_length;
   unsigned char ip_service_type;
   unsigned short ip_total_length;
   unsigned short ip_id;
   unsigned short ip_flags_fragment_offset;
   unsigned char ip_time_to_live;
   unsigned char ip_protocol;
   unsigned short ip_checksum;
   unsigned char ip_src_IP[4];
   unsigned char ip_dst_IP[4];
   
   /* ping��å�������40byte�� */
   unsigned char ping_type;
   unsigned char ping_code;
   unsigned short ping_checksum;
   unsigned short ping_id;
   unsigned short ping_sequence_number;
   unsigned char ping_data[32];
} PING_PACKET;

unsigned char rtl8019_read(unsigned char addr);
void rtl8019_write(unsigned char addr, unsigned char data);
void rtl8019_init(void);

void packet_send(unsigned char *packet, unsigned short size);
unsigned char packet_receive(unsigned char *packet);
void ARP_request(unsigned char *packet);
void ARP_reply(unsigned char *packet);

unsigned char str_to_IP(char *str, unsigned char *IP);
void MAC_to_str(unsigned char *MAC, char *str);
void IP_to_str(unsigned char *IP, char *str);
unsigned char IP_compare(unsigned char *IP_a, unsigned char *IP_b);
void io_regs_init();

static unsigned char src_MAC[6];
static unsigned char src_IP[4];
static unsigned char dst_MAC[6];
static unsigned char dst_IP[4];
static unsigned char packet[PACKET_MAX_NUM];

#endif