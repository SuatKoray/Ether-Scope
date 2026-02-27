#include "ether_scope.h"

// Programın çalışma durumunu kontrol eden global bayrak
volatile sig_atomic_t keep_running = 1;
FILE *pcap_file = NULL;

// İşletim sisteminden gelen sinyalleri yakalayan santral
void SignalHandler(int signum) {
    (void)signum; // Kullanılmayan parametre uyarısını sustur
    printf("\n" COLOR_RED "[!] Kapanis sinyali alindi (Ctrl+C). Guvenli bir sekilde kapaniyor..." COLOR_RESET "\n");
    keep_running = 0; // Döngüyü kır
}

int main() {
    int raw_sock;
    struct sockaddr_ll saddr;
    socklen_t saddr_len = sizeof(saddr);
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    if (buffer == NULL) {
        fprintf(stderr, "Bellek tahsis edilemedi!\n");
        return EXIT_FAILURE;
    }

    // SIGINT (Ctrl+C) sinyalini bizim SignalHandler fonksiyonumuza yönlendiriyoruz.
    signal(SIGINT, SignalHandler);

    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_sock < 0) {
        perror("Soket Hatası (Sudo kullandınız mı?)");
        free(buffer);
        return EXIT_FAILURE;
    }

    printf(COLOR_CYAN "====================================================" COLOR_RESET "\n");
    printf(COLOR_GREEN "  ETHER-SCOPE Aktif (L2, L3, L4 ve Payload Analizi)" COLOR_RESET "\n");
    
    // PCAP Dosyasını Başlat 
    pcap_file = InitPcapFile("capture.pcap");
    
    printf(COLOR_CYAN "====================================================" COLOR_RESET "\n");

    // "keep_running" bayrağı 1 olduğu sürece  döngü çalışır.
    while (keep_running) {
        ssize_t packet_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0, 
                                       (struct sockaddr *)&saddr, &saddr_len);

        // recvfrom kesintiye uğrarsa döngüden çık
        if (packet_size < 0) {
            if (errno == EINTR) break; 
            fprintf(stderr, "Paket yakalama hatası: %s\n", strerror(errno));
            break;
        }

        // Paketi analiz et ve ekrana bas
        ProcessPacket(buffer, packet_size);
        
        // Paketi PCAP dosyasına yaz
        WritePcapPacket(pcap_file, buffer, packet_size);
    }

    // --- GRACEFUL SHUTDOWN ---
    printf("[INFO] Kaynaklar temizleniyor...\n");
    if (pcap_file) {
        fclose(pcap_file);
        printf("[INFO] PCAP dosyasi 'capture.pcap' basariyla kaydedildi.\n");
    }
    close(raw_sock);
    free(buffer);
    printf("[INFO] Ether-Scope basariyla kapatildi.\n");
    
    return EXIT_SUCCESS;
}