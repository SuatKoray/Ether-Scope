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

#define BUFFER_SIZE 65536

/**
 * Ether-Scope: Düşük Seviyeli Paket Dinleyici
 * 1. AŞAMA: Soket Kurulumu ve Ham Veri Yakalama
 */

int main() {
    int raw_sock;
    struct sockaddr_ll saddr;
    socklen_t saddr_len = sizeof(saddr);
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    if (buffer == NULL) {
        fprintf(stderr, "Bellek tahsis edilemedi!\n");
        return EXIT_FAILURE;
    }

    // 1. ADIM: Raw Socket Oluşturma
    // ETH_P_ALL: Tüm protokolleri (IPv4, IPv6, ARP vb.) Layer 2 seviyesinde yakalar.
    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (raw_sock < 0) {
        perror("Soket Hatası");
        fprintf(stderr, "NOT: Raw socket açmak için 'sudo' (root) yetkisi gereklidir.\n");
        free(buffer);
        return EXIT_FAILURE;
    }

    printf("[INFO] Ether-Scope Aktif. Paketler dinleniyor...\n");
    printf("----------------------------------------------\n");

    while (1) {
        // 2. ADIM: Ham veriyi yakalama
        ssize_t packet_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0, 
                                       (struct sockaddr *)&saddr, &saddr_len);

        if (packet_size < 0) {
            fprintf(stderr, "Paket yakalama hatası: %s\n", strerror(errno));
            break;
        }

        // PROFESYONEL ÇIKTI: Şimdilik sadece boyut ve arayüz indeksi
        printf("Paket Yakalandı! Boyut: %ld byte | Interface Index: %d\n", 
                packet_size, saddr.sll_ifindex);
    }

    // Temizlik
    close(raw_sock);
    free(buffer);
    return EXIT_SUCCESS;
}