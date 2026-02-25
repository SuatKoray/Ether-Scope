#ifndef ETHER_SCOPE_H
#define ETHER_SCOPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/* Ağ kütüphaneleri */
#include <sys/socket.h>
#include <arpa/inet.h>       
#include <net/ethernet.h>    
#include <linux/if_packet.h> 
#include <linux/if_ether.h>  
#include <linux/ip.h>        
#include <linux/tcp.h>       
#include <linux/udp.h>       

#define BUFFER_SIZE 65536

/* ANSI Renk Kodları */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RED     "\x1b[31m"

/* Fonksiyon Prototipleri */
void ProcessPacket(unsigned char *buffer, ssize_t size);
void PrintEthernetHeader(unsigned char *buffer);
void PrintIpHeader(unsigned char *buffer, ssize_t total_size);
void PrintTcpPacket(unsigned char *buffer, int iphdr_len, ssize_t total_size);
void PrintUdpPacket(unsigned char *buffer, int iphdr_len, ssize_t total_size);
void PrintHexDump(const unsigned char *data, int size);

#endif // ETHER_SCOPE_H