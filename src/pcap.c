#include "ether_scope.h"

// PCAP dosyasını başlatır ve Global Başlığı (24 byte) yazar.
FILE* InitPcapFile(const char *filename) {
    FILE *f = fopen(filename, "wb"); // "wb": Write Binary
    if (!f) {
        perror("PCAP dosyasi olusturulamadi");
        return NULL;
    }

    pcap_hdr_t global_hdr;
    global_hdr.magic_number = 0xa1b2c3d4; // Wireshark bu imzayı arar
    global_hdr.version_major = 2;
    global_hdr.version_minor = 4;
    global_hdr.thiszone = 0;
    global_hdr.sigfigs = 0;
    global_hdr.snaplen = 65535;
    global_hdr.network = 1; // 1 = LINKTYPE_ETHERNET

    fwrite(&global_hdr, sizeof(pcap_hdr_t), 1, f);
    printf(COLOR_GREEN "[+] PCAP Kaydi Basladi: %s" COLOR_RESET "\n", filename);
    return f;
}

// Her yakalanan paketi, zaman damgasıyla birlikte dosyaya yazar.
void WritePcapPacket(FILE *pcap_file, unsigned char *buffer, ssize_t size) {
    if (!pcap_file) return;

    struct timeval tv;
    gettimeofday(&tv, NULL); // Paketin yakalandığı tam anı al

    pcaprec_hdr_t pkt_hdr;
    pkt_hdr.ts_sec = tv.tv_sec;
    pkt_hdr.ts_usec = tv.tv_usec;
    pkt_hdr.incl_len = size;
    pkt_hdr.orig_len = size;

    // 1. Önce paketin başlığını (16 byte) yaz
    fwrite(&pkt_hdr, sizeof(pcaprec_hdr_t), 1, pcap_file);
    // 2. Sonra doğrudan kernelden gelen ham 'buffer'ı yaz
    fwrite(buffer, size, 1, pcap_file);
    
    // Verinin tamponda kalmayıp hemen diske yazılmasını zorla
    fflush(pcap_file); 
}