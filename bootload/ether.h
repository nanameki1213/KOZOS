#ifndef _ETHER_H_INCLUDED_
#define _ETHER_H_INCLUDED_

unsigned char rtl8019_read(unsigned char addr);
void rtl8019_write(unsigned char addr, unsigned char data);
void rtl8019_init();

#endif