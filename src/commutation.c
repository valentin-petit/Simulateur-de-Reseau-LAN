#include "commutation.h"
#include <stdio.h>

// Ajoute une MAC à la table si pas déjà là, ou met à jour le port si besoin
int switch_apprendre_mac(switch_t *sw, mac_addr_t mac_src, int port) {
    int i;
    for (i = 0; i < sw->mac_table_size; i++) {
        if (mac_egal(sw->mac_table[i], mac_src)) {
            if (sw->port_table[i] != port) {
                sw->port_table[i] = port;
            }
            return i;
        }
    }
    if (sw->mac_table_size < MAX_PORTS) {
        sw->mac_table[sw->mac_table_size] = mac_src;
        sw->port_table[sw->mac_table_size] = port;
        sw->mac_table_size++;
        return sw->mac_table_size-1;
    }
    return -1; // table pleine
}

// Cherche le port associé à une MAC, -1 si inconnu
int switch_rechercher_port(switch_t *sw, mac_addr_t mac_dest) {
    int i;
    for (i = 0; i < sw->mac_table_size; i++) {
        if (mac_egal(sw->mac_table[i], mac_dest)) {
            return sw->port_table[i];
        }
    }
    return -1;
}

// Affiche la table MAC d'un switch
void afficher_table_mac(switch_t *sw) {
    printf("\n\033[1;36m=== Table MAC du Switch ===\033[0m\n");
    if (sw->mac_table_size == 0) {
        printf("\033[1;33m  Table vide\033[0m\n");
        return;
    }
    
    printf("\033[1;33m  Port  |  Adresse MAC        | État\033[0m\n");
    printf("\033[1;33m  ----- | ------------------- | -----\033[0m\n");
    
    for (int i = 0; i < sw->mac_table_size; i++) {
        printf("  %-5d | ", sw->port_table[i]);
        for (int j = 0; j < MAC_ADDR_LEN; j++) {
            printf("%02x", sw->mac_table[i].addr[j]);
            if (j < MAC_ADDR_LEN-1) printf(":");
        }
        printf(" | %s\n", sw->port_etat[i] ? "\033[1;32mActif\033[0m" : "\033[1;31mBloqué\033[0m");
    }
}
