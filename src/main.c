#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Ağ kütüphaneleri */
#include <sys/socket.h>
#include <arpa/inet.h>       // ntohs() fonksiyonu için
#include <net/ethernet.h>    // ETH_P_ALL ve diğer Ethernet sabitleri için
#include <linux/if_packet.h>
#include <linux/if_ether.h>  // struct ethhdr (Ethernet Başlığı) için

#define BUFFER_SIZE 65536

/* --- FONKSİYON PROTOTİPLERİ --- */
void ProcessPacket(unsigned char *buffer, ssize_t size);
void PrintEthernetHeader(unsigned char *buffer);

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

    printf("[INFO] Ether-Scope Aktif. L2 Paketleri Dinleniyor...\n");
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

/**
 * Gelen her paketin ilk uğradığı santral fonksiyonudur.
 */
void ProcessPacket(unsigned char *buffer, ssize_t size) {
    // Şimdilik sadece Ethernet başlığını ekrana basıyoruz
    // İleride buraya "Eğer IPv4 ise IP'yi parse et" mantığını kuracağız.
    PrintEthernetHeader(buffer);
    printf("   |-Paket Boyutu: %ld byte\n", size);
    printf("----------------------------------------------------\n");
}

/**
 * Ham buffer'ı alır, Ethernet başlığına (14 byte) cast eder ve bilgileri basar.
 */
void PrintEthernetHeader(unsigned char *buffer) {
    // Pointer Casting
    // buffer'ın gösterdiği adresi, bir 'ethhdr' yapısı (template) olarak okuyoruz.
    struct ethhdr *eth = (struct ethhdr *)buffer;

    printf("\n[*] ETHERNET CERCEVESI YAKALANDI\n");
    
    // Hedef ve Kaynak MAC adreslerini %.2X (Hexadecimal, 2 hane, büyük harf) formatında basıyoruz.
    printf("   |-Hedef MAC   : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", 
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], 
            eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
            
    printf("   |-Kaynak MAC  : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", 
            eth->h_source[0], eth->h_source[1], eth->h_source[2], 
            eth->h_source[3], eth->h_source[4], eth->h_source[5]);
            
    // Protokol tipini yazdırırken ntohs() kullanmak ZORUNLU.
    printf("   |-Protokol    : 0x%.4x\n", ntohs(eth->h_proto));
}