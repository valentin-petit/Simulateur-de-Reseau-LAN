#include "equipement.h"
#include <stdio.h>

// Affiche une adresse MAC en notation hexadécimale
void afficher_mac(mac_addr_t mac) {
    printf("\033[1;32m");
    for (int i = 0; i < MAC_ADDR_LEN; i++) {
        printf("%02x", mac.addr[i]);
        if (i < MAC_ADDR_LEN-1) printf(":");
    }
    printf("\033[0m");
}

// Affiche une adresse IP en notation décimale pointée
void afficher_ip(ip_addr_t ip) {
    printf("\033[1;32m");
    for (int i = 0; i < IP_ADDR_LEN; i++) {
        printf("%d", ip.addr[i]);
        if (i < IP_ADDR_LEN-1) printf(".");
    }
    printf("\033[0m");
}

// Affichage d'une station
void afficher_station(station_t s) {
    printf("\033[1;36m=== Station ===\033[0m\n");
    printf("\033[1;33mMAC : \033[0m");
    afficher_mac(s.mac);
    printf("\n\033[1;33mIP  : \033[0m");
    afficher_ip(s.ip);
    printf("\n");
}

// Affichage d'un switch
void afficher_switch(switch_t sw) {
    printf("\033[1;36m=== Switch ===\033[0m\n");
    printf("\033[1;33mMAC      : \033[0m");
    afficher_mac(sw.mac);
    printf("\n\033[1;33mPorts    : \033[0m%d\n", sw.nb_ports);
    printf("\033[1;33mPriorité : \033[0m%d\n", sw.priority);
    
    if (sw.mac_table_size > 0) {
        printf("\n\033[1;33mTable MAC :\033[0m\n");
        for (int i = 0; i < sw.mac_table_size; i++) {
            printf("  Port %-3d : ", sw.port_table[i]);
            afficher_mac(sw.mac_table[i]);
            printf(" [%s]\n", sw.port_etat[i] ? "\033[1;32mActif\033[0m" : "\033[1;31mBloqué\033[0m");
        }
    }
}

// Affichage générique d'un équipement
void afficher_equipement(equipement_t eq) {
    if (eq.type == STATION) {
        printf("\033[1;35m[Station]\033[0m ");
        afficher_station(eq.data.station);
    } else {
        printf("\033[1;35m[Switch]\033[0m ");
        afficher_switch(eq.data.sw);
    }
}
