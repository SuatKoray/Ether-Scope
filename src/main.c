#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Ağ kütüphaneleri */
#include <sys/socket.h>
#include <arpa/inet.h>       // ntohs(), inet_ntoa() için
#include <net/ethernet.h>    // ETH_P_ALL
#include <linux/if_packet.h> // sockaddr_ll
#include <linux/if_ether.h>  // struct ethhdr (Ethernet Başlığı)
#include <linux/ip.h>        // struct iphdr (IP Başlığı)

#define BUFFER_SIZE 65536

/* ANSI Renk Kodları (Profesyonel Terminal Çıktısı İçin) */
#define COLOR_RESET  "\x1b[0m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_BLUE   "\x1b[34m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN   "\x1b[36m"

/* --- FONKSİYON PROTOTİPLERİ --- */
void ProcessPacket(unsigned char *buffer, ssize_t size);
void PrintEthernetHeader(unsigned char *buffer);
void PrintIpHeader(unsigned char *buffer);

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

    printf("[INFO] Ether-Scope Aktif. L2 ve L3 Paketleri Dinleniyor...\n");
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
    
    PrintEthernetHeader(buffer);
    
    // YENİ: Eğer protokol IPv4 ise (Hex: 0x0800), IP başlığını ayrıştır!
    // ntohs() kullanımı kritik: Ağdan gelen Big-Endian veriyi kendi sistemimize (Little-Endian) çevirir.
    if (ntohs(eth->h_proto) == 0x0800) {
        PrintIpHeader(buffer);
    }
    
    printf("   |-Paket Boyutu: %ld byte\n", size);
    printf("----------------------------------------------------\n");
}

void PrintEthernetHeader(unsigned char *buffer) {
    struct ethhdr *eth = (struct ethhdr *)buffer;

    printf("\n[*] ETHERNET CERCEVESI YAKALANDI\n");
    printf("   |-Hedef MAC   : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", 
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], 
            eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
            
    printf("   |-Kaynak MAC  : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", 
            eth->h_source[0], eth->h_source[1], eth->h_source[2], 
            eth->h_source[3], eth->h_source[4], eth->h_source[5]);
            
    printf("   |-Protokol    : 0x%.4x\n", ntohs(eth->h_proto));
}

void PrintIpHeader(unsigned char *buffer) {
    // POINTER ARİTMETİĞİ 
    // buffer'ın başından (0. byte) tam olarak sizeof(struct ethhdr) yani 14 byte ileri atlıyoruz.
    // Artık IP başlığının tam başladığı noktadayız.
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    
    struct sockaddr_in source, dest;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;

    printf("\n" COLOR_CYAN "[*] IPv4 BASLIGI YAKALANDI" COLOR_RESET "\n");
    printf("   |-Kaynak IP   : %s\n", inet_ntoa(source.sin_addr));
    printf("   |-Hedef IP    : %s\n", inet_ntoa(dest.sin_addr));
    printf("   |-TTL (Yasam) : %d\n", (unsigned int)ip->ttl);
    
    // IP başlığındaki 'protocol' alanı, paketin içindeki verinin tipini söyler (Örn: 6 = TCP, 17 = UDP)
    printf("   |-Protokol    : ");
    switch (ip->protocol) {
        case 1:  
            printf(COLOR_YELLOW "ICMP" COLOR_RESET "\n"); 
            break;
        case 6:  
            printf(COLOR_GREEN "TCP" COLOR_RESET "\n"); 
            break;
        case 17: 
            printf(COLOR_BLUE "UDP" COLOR_RESET "\n"); 
            break;
        default: 
            printf("Diger (%d)\n", ip->protocol); 
            break;
    }
}