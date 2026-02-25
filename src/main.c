#include "ether_scope.h"

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
        perror("Soket Hatası (Sudo kullandınız mı?)");
        free(buffer);
        return EXIT_FAILURE;
    }

    printf("[INFO] Ether-Scope Aktif. Ham Veri (Payload) Analizi Devrede...\n");
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