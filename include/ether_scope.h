#ifndef ETHER_SCOPE_H
#define ETHER_SCOPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>      // Sinyal yakalama (Ctrl+C)
#include <sys/time.h>    // Mikrosaniye hassasiyetinde zaman (PCAP için)
#include <stdint.h>      // Sabit boyutlu tam sayılar (uint32_t vb.)

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

/* --- PCAP STRUCT YAPILARI (Wireshark Standardı) --- */
// PCAP Global Başlığı (Dosya başına 1 kez yazılır)
typedef struct pcap_hdr_s {
    uint32_t magic_number;   /* Sihirli numara: 0xa1b2c3d4 */
    uint16_t version_major;  /* Major sürüm: 2 */
    uint16_t version_minor;  /* Minor sürüm: 4 */
    int32_t  thiszone;       /* GMT zaman dilimi sapması */
    uint32_t sigfigs;        /* Zaman damgası hassasiyeti */
    uint32_t snaplen;        /* Maksimum paket uzunluğu */
    uint32_t network;        /* Ağ katmanı tipi (Ethernet = 1) */
} pcap_hdr_t;

// PCAP Paket Başlığı 
typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;         /* Zaman damgası: Saniye */
    uint32_t ts_usec;        /* Zaman damgası: Mikrosaniye */
    uint32_t incl_len;       /* Dosyaya yazılan boyut */
    uint32_t orig_len;       /* Paketin asıl boyutu */
} pcaprec_hdr_t;

/* --- Fonksiyon Prototipleri --- */
void ProcessPacket(unsigned char *buffer, ssize_t size);
void PrintEthernetHeader(unsigned char *buffer);
void PrintIpHeader(unsigned char *buffer, ssize_t total_size);
void PrintTcpPacket(unsigned char *buffer, int iphdr_len, ssize_t total_size);
void PrintUdpPacket(unsigned char *buffer, int iphdr_len, ssize_t total_size);
void PrintHexDump(const unsigned char *data, int size);

/* PCAP Fonksiyonları */
FILE* InitPcapFile(const char *filename);
void WritePcapPacket(FILE *pcap_file, unsigned char *buffer, ssize_t size);

#endif // ETHER_SCOPE_H