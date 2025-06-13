#include "equipement.h"
#include "affichage.h"
#include "reseau.h"
#include "trame.h"
#include "commutation.h"
#include "stp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_MAX_EQUIPEMENTS 64


// Recherche d'une station par IP
int trouver_station_par_ip(reseau_t *reseau, ip_addr_t ip) {
    int i;
    for (i = 0; i < reseau->nb_equipements; i++) {
        if (reseau->equipements[i].type == STATION &&
            ip_egal(reseau->equipements[i].data.station.ip, ip)) {
            return i;
        }
    }
    return -1;
}

// Propagation de la trame avec anti-boucle simple
int propager_trame(
    reseau_t *reseau,
    int courant,
    int precedent,
    const ethernet_frame_t *trame,
    int dest_station,
    int *trouve,
    int profondeur,
    int *visited
) {
    if (profondeur > reseau->nb_equipements) return 0;
    if (*trouve) return 1;
    if (visited[courant]) return 0;
    visited[courant] = 1;

    if (courant == dest_station) {
        printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[1;36m║\033[0m \033[1;32m🎯 Trame reçue !\033[0m%*s\033[1;36m║\033[0m\n", 45, "");
        printf("\033[1;36m║\033[0m \033[1;33mStation destination :\033[0m %d%*s\033[1;36m║\033[0m\n", courant, 35, "");
        printf("\033[1;36m║\033[0m \033[1;33mMAC destination    :\033[0m ");
        afficher_mac(trame->dest);
        printf("%*s\033[1;36m║\033[0m\n", 15, "");
        printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
        *trouve = 1;
        return 1;
    }

    equipement_t *eq = &reseau->equipements[courant];
    if (eq->type == SWITCH) {
        switch_t *sw = &eq->data.sw;
        if (precedent != -1) {
            int port_enregistre = switch_rechercher_port(sw, trame->src);
            if (port_enregistre == -1) {
                printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
                printf("\033[1;36m║\033[0m \033[1;33m📝 Switch %d apprend une nouvelle adresse MAC\033[0m%*s\033[1;36m║\033[0m\n", courant, 15, "");
                printf("\033[1;36m║\033[0m \033[1;33mMAC source :\033[0m ");
                afficher_mac(trame->src);
                printf("%*s\033[1;36m║\033[0m\n", 25, "");
                printf("\033[1;36m║\033[0m \033[1;33mPort      :\033[0m %d%*s\033[1;36m║\033[0m\n", precedent, 40, "");
                printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
                switch_apprendre_mac(sw, trame->src, precedent);
            }
        }

        int port_equip = switch_rechercher_port(sw, trame->dest);
        if (port_equip != -1 && port_equip != precedent) {
            printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
            printf("\033[1;36m║\033[0m \033[1;33m🔄 Switch %d : MAC destination connue\033[0m%*s\033[1;36m║\033[0m\n", courant, 25, "");
            printf("\033[1;36m║\033[0m \033[1;33mEnvoi vers l'équipement :\033[0m %d%*s\033[1;36m║\033[0m\n", port_equip, 30, "");
            printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
            propager_trame(reseau, port_equip, courant, trame, dest_station, trouve, profondeur + 1, visited);
            return 1;
        } else {
            printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
            printf("\033[1;36m║\033[0m \033[1;33m🌊 Switch %d : MAC destination inconnue\033[0m%*s\033[1;36m║\033[0m\n", courant, 25, "");
            printf("\033[1;36m║\033[0m \033[1;33mInondation de tous les ports actifs\033[0m%*s\033[1;36m║\033[0m\n", 20, "");
            printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
            int i;
            for (i = 0; i < reseau->nb_liens; i++) {
                int e1 = reseau->liens[i].equip1, e2 = reseau->liens[i].equip2;
                int voisin = (e1 == courant) ? e2 : (e2 == courant ? e1 : -1);
                if (voisin != -1 && voisin != precedent) {
                    propager_trame(reseau, voisin, courant, trame, dest_station, trouve, profondeur + 1, visited);
                    if (*trouve) return 1;
                }
            }
        }
    }
    return 0;
}

void decoder_donnees(const uint8_t *data, uint16_t data_len) {
    printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m📦 Décodage des données reçues\033[0m%*s\033[1;36m║\033[0m\n", 35, "");
    printf("\033[1;36m╟────────────────────────────────────────────────────────────────╢\033[0m\n");
    
    // Affichage en hexadécimal
    printf("\033[1;36m║\033[0m \033[1;33mHexadécimal :\033[0m ");
    for (int i = 0; i < data_len; i++) {
        printf("%02x ", data[i]);
    }
    printf("%*s\033[1;36m║\033[0m\n", 40 - (data_len * 3), "");
    
    // Affichage en ASCII
    printf("\033[1;36m║\033[0m \033[1;33mASCII      :\033[0m ");
    for (int i = 0; i < data_len; i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    printf("%*s\033[1;36m║\033[0m\n", 40 - data_len, "");
    
    // Affichage en décimal
    printf("\033[1;36m║\033[0m \033[1;33mDécimal    :\033[0m ");
    for (int i = 0; i < data_len; i++) {
        printf("%d ", data[i]);
    }
    printf("%*s\033[1;36m║\033[0m\n", 40 - (data_len * 4), "");
    
    printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
}

// Simulation d'une trame entre deux stations
void simuler_trame_station(reseau_t *reseau, int idx_src, int idx_dest) {
    printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m🚀 Démarrage de la simulation\033[0m%*s\033[1;36m║\033[0m\n", 35, "");
    printf("\033[1;36m║\033[0m \033[1;33mSource      :\033[0m Station %d%*s\033[1;36m║\033[0m\n", idx_src, 35, "");
    printf("\033[1;36m║\033[0m \033[1;33mDestination :\033[0m Station %d%*s\033[1;36m║\033[0m\n", idx_dest, 35, "");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");

    if (reseau->equipements[idx_src].type != STATION || reseau->equipements[idx_dest].type != STATION) {
        printf("\n\033[1;31m❌ Erreur: L'indice source ou destination ne correspond pas à une station.\033[0m\n");
        return;
    }

    station_t src = reseau->equipements[idx_src].data.station;
    station_t dest = reseau->equipements[idx_dest].data.station;

    // Saisie des données par l'utilisateur
    char data_str[ETHERNET_MAX_DATA * 2 + 1];
    uint8_t data[ETHERNET_MAX_DATA];
    uint16_t data_len = 0;

    printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m📝 Saisie des données à envoyer\033[0m%*s\033[1;36m║\033[0m\n", 35, "");
    printf("\033[1;36m║\033[0m \033[1;33mEntrez les données (max %d octets) :\033[0m%*s\033[1;36m║\033[0m\n", ETHERNET_MAX_DATA, 15, "");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\033[1;33m> \033[0m");
    
    if (fgets(data_str, sizeof(data_str), stdin) != NULL) {
        data_str[strcspn(data_str, "\n")] = 0; // Supprime le retour à la ligne
        data_len = strlen(data_str);
        if (data_len > ETHERNET_MAX_DATA) {
            data_len = ETHERNET_MAX_DATA;
        }
        memcpy(data, data_str, data_len);
    }

    ethernet_frame_t trame;
    creer_trame_ethernet(&trame, src.mac, dest.mac, 0x0800, data, data_len);
    
    printf("\n\033[1;36m=== Trame à envoyer ===\033[0m\n");
    afficher_trame_utilisateur(&trame);

    int trouve = 0;
    int visited[NB_MAX_EQUIPEMENTS] = {0};
    int i;
    for (i = 0; i < reseau->nb_liens; i++) {
        int e1 = reseau->liens[i].equip1, e2 = reseau->liens[i].equip2;
        int voisin = (e1 == idx_src) ? e2 : (e2 == idx_src ? e1 : -1);
        if (voisin != -1) {
            propager_trame(reseau, voisin, idx_src, &trame, idx_dest, &trouve, 1, visited);
            if (trouve) {
                // Décodage des données reçues
                decoder_donnees(trame.data, trame.data_len);
                break;
            }
        }
    }
    if (!trouve) {
        printf("\n\033[1;31m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[1;31m║\033[0m \033[1;31m❌ La trame n'a pas pu atteindre la station destination\033[0m%*s\033[1;31m║\033[0m\n", 10, "");
        printf("\033[1;31m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
    }
}

// --- Menu Display Function ---

void afficher_menu_principal() {
    printf("\n\033[1;36m╔════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║           MENU PRINCIPAL                ║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m1.\033[0m Lancer une simulation de trame      \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m2.\033[0m Afficher l'état des ports STP       \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m3.\033[0m Afficher les tables MAC             \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m4.\033[0m Afficher la matrice d'adjacence     \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m0.\033[0m Quitter                             \033[1;36m║\033[0m\n");
    printf("\033[1;36m╚════════════════════════════════════════╝\033[0m\n");
    printf("\n\033[1;33mVotre choix : \033[0m");
}

void afficher_stations_disponibles(reseau_t *reseau) {
    printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║                     STATIONS DISPONIBLES                         ║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33mIndex\033[0m | \033[1;33mAdresse MAC\033[0m           | \033[1;33mAdresse IP\033[0m              \033[1;36m║\033[0m\n");
    printf("\033[1;36m╟────────────────────────────────────────────────────────────────╢\033[0m\n");
    
    int nb_stations = 0;
    for (int i = 0; i < reseau->nb_equipements; i++) {
        if (reseau->equipements[i].type == STATION) {
            station_t *s = &reseau->equipements[i].data.station;
            printf("\033[1;36m║\033[0m %-5d | ", i);
            afficher_mac(s->mac);
            printf(" | ");
            afficher_ip(s->ip);
            printf("%*s\033[1;36m║\033[0m\n", 15 - (IP_ADDR_LEN * 4), "");
            nb_stations++;
        }
    }
    
    if (nb_stations == 0) {
        printf("\033[1;36m║\033[0m \033[1;31mAucune station disponible\033[0m%*s\033[1;36m║\033[0m\n", 45, "");
    }
    
    printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
}

void afficher_matrice_adjacence(reseau_t *reseau) {
    printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m \033[1;33m📊 Matrice d'adjacence du réseau\033[0m%*s\033[1;36m║\033[0m\n", 35, "");
    printf("\033[1;36m╟────────────────────────────────────────────────────────────────╢\033[0m\n");
    
    // En-tête avec les numéros des équipements
    printf("\033[1;36m║\033[0m     ");
    for (int i = 0; i < reseau->nb_equipements; i++) {
        printf("%2d ", i);
    }
    printf("%*s\033[1;36m║\033[0m\n", 40 - (reseau->nb_equipements * 3), "");
    
    // Ligne de séparation
    printf("\033[1;36m║\033[0m   ─");
    for (int i = 0; i < reseau->nb_equipements; i++) {
        printf("───");
    }
    printf("%*s\033[1;36m║\033[0m\n", 40 - (reseau->nb_equipements * 3), "");
    
    // Contenu de la matrice
    for (int i = 0; i < reseau->nb_equipements; i++) {
        printf("\033[1;36m║\033[0m %2d │", i);
        for (int j = 0; j < reseau->nb_equipements; j++) {
            int poids = -1;
            for (int k = 0; k < reseau->nb_liens; k++) {
                if ((reseau->liens[k].equip1 == i && reseau->liens[k].equip2 == j) ||
                    (reseau->liens[k].equip1 == j && reseau->liens[k].equip2 == i)) {
                    poids = reseau->liens[k].poids;
                    break;
                }
            }
            if (poids == -1) {
                printf(" \033[1;31m.\033[0m ");
            } else {
                printf(" \033[1;32m%d\033[0m ", poids);
            }
        }
        printf("%*s\033[1;36m║\033[0m\n", 40 - (reseau->nb_equipements * 3), "");
    }
    
    printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n");
    
    // Légende
    printf("\n\033[1;33mLégende :\033[0m\n");
    printf("  \033[1;32m[chiffre]\033[0m : Poids de la liaison entre les équipements\n");
    printf("  \033[1;31m.\033[0m : Équipements non connectés\n");
}

// --- Main Function ---

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("\033[1;31mUsage: %s <fichier_config>\033[0m\n", argv[0]);
        return EXIT_FAILURE;
    }

    reseau_t reseau;
    reseau.nb_equipements = 0;
    reseau.nb_liens = 0;

    printf("\033[1;36mChargement du réseau depuis le fichier '%s'...\033[0m\n", argv[1]);
    if (charger_reseau(argv[1], &reseau) != 0) {
        printf("\033[1;31mErreur lors du chargement du fichier réseau.\033[0m\n");
        return EXIT_FAILURE;
    }
    printf("\033[1;32mRéseau chargé avec succès !\033[0m\n");

    printf("\n\033[1;36m=== Calcul du Spanning Tree Protocol (STP) ===\033[0m\n");
    stp_calculer_spanning_tree(&reseau);
    printf("\n\033[1;36m=== État initial des ports après STP ===\033[0m\n");
    stp_afficher_etat_ports(&reseau);

    int choix_menu_principal = -1;

    while (choix_menu_principal != 0) {
        afficher_menu_principal();
        if (scanf("%d", &choix_menu_principal) != 1) {
            fprintf(stderr, "\033[1;31mEntrée invalide. Veuillez entrer un nombre.\033[0m\n");
            choix_menu_principal = -1;
            while (getchar() != '\n');
            continue;
        }
        getchar();

        switch (choix_menu_principal) {
            case 1: { // Lancer une simulation de trame (choix des stations)
                int idx_src, idx_dest;

                printf("\n\033[1;36m=== Lancement d'une simulation de trame ===\033[0m\n");
                afficher_stations_disponibles(&reseau);

                printf("\n\033[1;33mSélection de la station source :\033[0m\n");
                printf("\033[1;33mEntrez l'index de la station source : \033[0m");
                if (scanf("%d", &idx_src) != 1) {
                    fprintf(stderr, "\033[1;31mEntrée invalide pour l'index source.\033[0m\n");
                    while (getchar() != '\n');
                    break;
                }
                getchar();

                if (idx_src < 0 || idx_src >= reseau.nb_equipements || reseau.equipements[idx_src].type != STATION) {
                    printf("\033[1;31mErreur: L'index source %d n'est pas valide ou ne correspond pas à une station.\033[0m\n", idx_src);
                    break;
                }

                printf("\n\033[1;33mSélection de la station destination :\033[0m\n");
                printf("\033[1;33mEntrez l'index de la station destination : \033[0m");
                if (scanf("%d", &idx_dest) != 1) {
                    fprintf(stderr, "\033[1;31mEntrée invalide pour l'index destination.\033[0m\n");
                    while (getchar() != '\n');
                    break;
                }
                getchar();

                if (idx_dest < 0 || idx_dest >= reseau.nb_equipements || reseau.equipements[idx_dest].type != STATION) {
                    printf("\033[1;31mErreur: L'index destination %d n'est pas valide ou ne correspond pas à une station.\033[0m\n", idx_dest);
                    break;
                }

                if (idx_src == idx_dest) {
                    printf("\033[1;31mErreur: La station source et la station destination sont identiques.\033[0m\n");
                    break;
                }

                printf("\n\033[1;36m=== Détails de la simulation ===\033[0m\n");
                printf("\033[1;33mSource      : \033[0mStation %d (", idx_src);
                afficher_mac(reseau.equipements[idx_src].data.station.mac);
                printf(")\n");
                printf("\033[1;33mDestination : \033[0mStation %d (", idx_dest);
                afficher_mac(reseau.equipements[idx_dest].data.station.mac);
                printf(")\n\n");

                simuler_trame_station(&reseau, idx_src, idx_dest);

                printf("\n\033[1;36m=== TABLES MAC APRÈS LA SIMULATION ===\033[0m\n");
                for (int i = 0; i < reseau.nb_equipements; i++) {
                    if (reseau.equipements[i].type == SWITCH) {
                        printf("\n\033[1;35mSwitch %d :\033[0m\n", i);
                        afficher_table_mac(&reseau.equipements[i].data.sw);
                    }
                }
                break;
            }
            case 2: // Afficher l'état des ports STP
                printf("\n\033[1;36m=== État des ports STP ===\033[0m\n");
                stp_afficher_etat_ports(&reseau);
                break;
            case 3: // Afficher les tables MAC de tous les switches
                printf("\n\033[1;36m=== TABLES MAC DE TOUS LES SWITCHES ===\033[0m\n");
                for (int i = 0; i < reseau.nb_equipements; i++) {
                    if (reseau.equipements[i].type == SWITCH) {
                        printf("\n\033[1;35mSwitch %d :\033[0m\n", i);
                        afficher_table_mac(&reseau.equipements[i].data.sw);
                    }
                }
                break;
            case 4:
                afficher_matrice_adjacence(&reseau);
                break;
            case 0: // Quitter
                printf("\n\033[1;32mAu revoir !\033[0m\n");
                break;
            default:
                printf("\033[1;31mChoix invalide. Veuillez réessayer.\033[0m\n");
                break;
        }
    }
    return EXIT_SUCCESS;
}
