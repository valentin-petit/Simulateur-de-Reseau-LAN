#include "trame.h"
#include <stdio.h>

// Construction d'une trame Ethernet simple
void creer_trame_ethernet(
    ethernet_frame_t *trame,
    mac_addr_t src,
    mac_addr_t dest,
    uint16_t type,
    const uint8_t *data,
    uint16_t data_len
) {
    int i;
    for (i = 0; i < 7; i++) trame->preambule[i] = 0xAA;
    trame->sfd = 0xAB;
    trame->dest = dest;
    trame->src = src;
    trame->type = type;

    // Copie manuelle des données
    if (data_len > ETHERNET_MAX_DATA) data_len = ETHERNET_MAX_DATA;
    for (i = 0; i < data_len; i++) trame->data[i] = data[i];
    trame->data_len = data_len;

    // Padding si data < 46 octets
    int padding = (data_len < 46) ? (46 - data_len) : 0;
    for (i = 0; i < padding; i++) trame->bourrage[i] = 0;

    // FCS fictif
    trame->fcs = 0xDEADBEEF;
}

// Affiche une trame de façon lisible
void afficher_trame_utilisateur(const ethernet_frame_t *trame) {
    int i;
    printf("\n\033[1;36m=== Trame Ethernet ===\033[0m\n");
    printf("\033[1;33m  Destination : \033[0m");
    afficher_mac(trame->dest);
    printf("\n\033[1;33m  Source      : \033[0m");
    afficher_mac(trame->src);
    printf("\n\033[1;33m  Type        : \033[0m0x%04x (%s)\n", trame->type, 
           trame->type == 0x0800 ? "IPv4" : 
           trame->type == 0x0806 ? "ARP" : 
           trame->type == 0x86DD ? "IPv6" : "Inconnu");
    printf("\033[1;33m  Data length : \033[0m%d octets\n", trame->data_len);
    printf("\033[1;33m  Data        : \033[0m");
    for (i = 0; i < trame->data_len; i++) {
        printf("%02x ", trame->data[i]);
        if ((i + 1) % 16 == 0) printf("\n                 ");
    }
    printf("\n\033[1;33m  FCS         : \033[0m0x%08x\n", trame->fcs);
}


// Affiche la trame en hexadécimal (brut)
void afficher_trame_hex(const ethernet_frame_t *trame) {
    int i;
    printf("\n\033[1;36m=== Trame (hex) ===\033[0m\n");
    printf("\033[1;33mPréambule : \033[0m");
    for (i = 0; i < 7; i++) printf("%02x ", trame->preambule[i]);
    printf("\n\033[1;33mSFD       : \033[0m%02x\n", trame->sfd);
    printf("\033[1;33mDest      : \033[0m");
    for (i = 0; i < MAC_ADDR_LEN; i++) printf("%02x ", trame->dest.addr[i]);
    printf("\n\033[1;33mSource    : \033[0m");
    for (i = 0; i < MAC_ADDR_LEN; i++) printf("%02x ", trame->src.addr[i]);
    printf("\n\033[1;33mType      : \033[0m%02x %02x\n", (trame->type >> 8) & 0xFF, trame->type & 0xFF);
    printf("\033[1;33mData      : \033[0m");
    for (i = 0; i < trame->data_len; i++) {
        printf("%02x ", trame->data[i]);
        if ((i + 1) % 16 == 0) printf("\n          ");
    }
    int bourrage_len = (trame->data_len < 46) ? (46 - trame->data_len) : 0;
    if (bourrage_len > 0) {
        printf("\n\033[1;33mBourrage  : \033[0m");
        for (i = 0; i < bourrage_len; i++) printf("%02x ", trame->bourrage[i]);
    }
    printf("\n\033[1;33mFCS       : \033[0m%02x %02x %02x %02x\n",
        (trame->fcs >> 24) & 0xFF,
        (trame->fcs >> 16) & 0xFF,
        (trame->fcs >> 8) & 0xFF,
        trame->fcs & 0xFF);
}
