#include "ether_scope.h"

void PrintHexDump(const unsigned char *data, int size) {
    printf(COLOR_MAGENTA "[*] PAYLOAD (VERI) - Boyut: %d byte" COLOR_RESET "\n", size);
    
    if (size <= 0) {
        printf("   |-Paket icerisinde veri (payload) yok.\n");
        return;
    }

    for (int i = 0; i < size; i += 16) {
        printf("   %04X: ", i); 
        
        for (int j = 0; j < 16; j++) {
            if (i + j < size) printf("%02X ", data[i + j]);
            else printf("   "); 
        }
        
        printf(" | ");
        
        for (int j = 0; j < 16; j++) {
            if (i + j < size) {
                if (isprint(data[i + j])) printf("%c", data[i + j]);
                else printf(COLOR_RED "." COLOR_RESET); 
            }
        }
        printf("\n");
    }
}