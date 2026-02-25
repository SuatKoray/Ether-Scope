#include "ether_scope.h"

void ProcessPacket(unsigned char *buffer, ssize_t size) {
    struct ethhdr *eth = (struct ethhdr *)buffer;
    
    if (ntohs(eth->h_proto) == 0x0800) { // Sadece IPv4
        PrintEthernetHeader(buffer);
        PrintIpHeader(buffer, size);
        printf("   |-Toplam Paket Boyutu: %ld byte\n", size);
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

void PrintIpHeader(unsigned char *buffer, ssize_t total_size) {
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    unsigned short iphdr_len = ip->ihl * 4;

    struct sockaddr_in source, dest;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;

    printf(COLOR_CYAN "[*] IPv4 BASLIGI" COLOR_RESET "\n");
    printf("   |-Kaynak IP   : %s\n", inet_ntoa(source.sin_addr));
    printf("   |-Hedef IP    : %s\n", inet_ntoa(dest.sin_addr));
    
    switch (ip->protocol) {
        case 6:  
            PrintTcpPacket(buffer, iphdr_len, total_size);
            break;
        case 17: 
            PrintUdpPacket(buffer, iphdr_len, total_size);
            break;
        case 1:  
            printf(COLOR_YELLOW "[*] ICMP PAKETI" COLOR_RESET "\n");
            break;
        default: 
            printf("   |-Protokol    : Diger (%d)\n", ip->protocol); 
            break;
    }
}

void PrintTcpPacket(unsigned char *buffer, int iphdr_len, ssize_t total_size) {
    struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + iphdr_len);
    
    printf(COLOR_GREEN "[*] TCP BASLIGI" COLOR_RESET "\n");
    printf("   |-Kaynak Port : %u\n", ntohs(tcp->source));
    printf("   |-Hedef Port  : %u\n", ntohs(tcp->dest));

    int tcphdr_len = tcp->doff * 4;
    int header_size = sizeof(struct ethhdr) + iphdr_len + tcphdr_len;
    int payload_size = total_size - header_size;

    unsigned char *payload = buffer + header_size;
    PrintHexDump(payload, payload_size);
}

void PrintUdpPacket(unsigned char *buffer, int iphdr_len, ssize_t total_size) {
    struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + iphdr_len);

    printf(COLOR_BLUE "[*] UDP BASLIGI" COLOR_RESET "\n");
    printf("   |-Kaynak Port : %u\n", ntohs(udp->source));
    printf("   |-Hedef Port  : %u\n", ntohs(udp->dest));

    int header_size = sizeof(struct ethhdr) + iphdr_len + sizeof(struct udphdr);
    int payload_size = total_size - header_size;

    unsigned char *payload = buffer + header_size;
    PrintHexDump(payload, payload_size);
}