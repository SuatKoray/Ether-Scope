#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Ağ kütüphaneleri */
#include <sys/socket.h>
#include <arpa/inet.h>       
#include <net/ethernet.h>    
#include <linux/if_packet.h> 
#include <linux/if_ether.h>  
#include <linux/ip.h>        
#include <linux/tcp.h>       // YENİ: struct tcphdr (TCP Başlığı)
#include <linux/udp.h>       // YENİ: struct udphdr (UDP Başlığı)

#define BUFFER_SIZE 65536

/* ANSI Renk Kodları */
#define COLOR_RESET  "\x1b[0m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_BLUE   "\x1b[34m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN   "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"

/* --- FONKSİYON PROTOTİPLERİ --- */
void ProcessPacket(unsigned char *buffer, ssize_t size);
void PrintEthernetHeader(unsigned char *buffer);
void PrintIpHeader(unsigned char *buffer);
void PrintTcpPacket(unsigned char *buffer, int iphdr_len); // YENİ
void PrintUdpPacket(unsigned char *buffer, int iphdr_len); // YENİ

int main() {
    int raw_sock;
    struct sockaddr_ll saddr;
    socklen_t saddr_len = sizeof(saddr);
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    if (buffer == NULL) {
        fprintf(stderr, "Bellek tahsis edilemedi!\n");
        return EXIT_FAILURE;
    }

    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_sock < 0) {
        perror("Soket Hatası");
        free(buffer);
        return EXIT_FAILURE;
    }

    printf("[INFO] Ether-Scope Aktif. L2, L3 ve L4 Paketleri Dinleniyor...\n");
    printf("====================================================\n");

    while (1) {
        ssize_t packet_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0, 
                                       (struct sockaddr *)&saddr, &saddr_len);

        if (packet_size < 0) {
            fprintf(stderr, "Paket yakalama hatası: %s\n", strerror(errno));
            break;
        }

        ProcessPacket(buffer, packet_size);
    }

    close(raw_sock);
    free(buffer);
    return EXIT_SUCCESS;
}

/* --- FONKSİYON TANIMLAMALARI --- */

void ProcessPacket(unsigned char *buffer, ssize_t size) {
    struct ethhdr *eth = (struct ethhdr *)buffer;
    
    // Yalnızca IPv4 paketlerini tam analiz ediyoruz
    if (ntohs(eth->h_proto) == 0x0800) {
        PrintEthernetHeader(buffer);
        PrintIpHeader(buffer);
        printf("   |-Paket Boyutu: %ld byte\n", size);
        printf("----------------------------------------------------\n");
    }
}

void PrintEthernetHeader(unsigned char *buffer) {
    struct ethhdr *eth = (struct ethhdr *)buffer;

    printf("\n[*] ETHERNET CERCEVESI\n");
    printf("   |-Hedef MAC   : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", 
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], 
            eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
            
    printf("   |-Kaynak MAC  : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", 
            eth->h_source[0], eth->h_source[1], eth->h_source[2], 
            eth->h_source[3], eth->h_source[4], eth->h_source[5]);
}

void PrintIpHeader(unsigned char *buffer) {
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    
    struct sockaddr_in source, dest;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;

    // Dinamik IP Başlığı Uzunluğu (IHL)
    // ihl (Internet Header Length) alanı 32-bit (4 byte) kelimeler cinsindendir.
    // Bu yüzden gerçek byte uzunluğunu bulmak için 4 ile çarpıyoruz.
    unsigned short iphdr_len = ip->ihl * 4;

    printf(COLOR_CYAN "[*] IPv4 BASLIGI" COLOR_RESET "\n");
    printf("   |-Kaynak IP   : %s\n", inet_ntoa(source.sin_addr));
    printf("   |-Hedef IP    : %s\n", inet_ntoa(dest.sin_addr));
    printf("   |-TTL         : %d\n", (unsigned int)ip->ttl);
    
    // Taşıma Katmanına (L4) Yönlendirme
    switch (ip->protocol) {
        case 6:  // TCP
            PrintTcpPacket(buffer, iphdr_len);
            break;
        case 17: // UDP
            PrintUdpPacket(buffer, iphdr_len);
            break;
        case 1:  // ICMP
            printf(COLOR_YELLOW "[*] ICMP PAKETI YAKALANDI" COLOR_RESET "\n");
            break;
        default: 
            printf("   |-Protokol    : Diger (%d)\n", ip->protocol); 
            break;
    }
}

// YENİ: TCP Başlığını Ayrıştırma
void PrintTcpPacket(unsigned char *buffer, int iphdr_len) {
    // DİNAMİK SIKRAMA: buffer + Ethernet(14) + IP Başlığı(Dinamik)
    struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + iphdr_len);

    printf(COLOR_GREEN "[*] TCP BASLIGI" COLOR_RESET "\n");
    // Portlar ağ üzerinden geldiği için ntohs() ile çevrilmek zorundadır!
    printf("   |-Kaynak Port : %u\n", ntohs(tcp->source));
    printf("   |-Hedef Port  : %u\n", ntohs(tcp->dest));
}

// YENİ: UDP Başlığını Ayrıştırma
void PrintUdpPacket(unsigned char *buffer, int iphdr_len) {
    // DİNAMİK SIKRAMA: buffer + Ethernet(14) + IP Başlığı(Dinamik)
    struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + iphdr_len);

    printf(COLOR_BLUE "[*] UDP BASLIGI" COLOR_RESET "\n");
    printf("   |-Kaynak Port : %u\n", ntohs(udp->source));
    printf("   |-Hedef Port  : %u\n", ntohs(udp->dest));
}