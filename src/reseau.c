#include "reseau.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Convertit une chaîne de caractères au format MAC (xx:yy:zz:...) en structure mac_addr_t
mac_addr_t parse_mac(const char *str) {
    mac_addr_t mac;
    unsigned int bytes[MAC_ADDR_LEN];
    sscanf(str, "%x:%x:%x:%x:%x:%x",
           &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]);
    for (int i = 0; i < MAC_ADDR_LEN; i++) mac.addr[i] = (uint8_t)bytes[i];
    return mac;
}

// Convertit une chaîne de caractères au format IP (a.b.c.d) en structure ip_addr_t
ip_addr_t parse_ip(const char *str) {
    ip_addr_t ip;
    unsigned int bytes[IP_ADDR_LEN];
    sscanf(str, "%u.%u.%u.%u", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
    for (int i = 0; i < IP_ADDR_LEN; i++) ip.addr[i] = (uint8_t)bytes[i];
    return ip;
}

// Charge la configuration du réseau depuis un fichier texte
// Format attendu : nombre d'équipements et de liens sur la première ligne
// Suivi des descriptions des équipements (type;mac;ip pour les stations, type;mac;nb_ports;priority pour les switches)
// Et enfin les liens entre équipements (equip1;equip2;poids)
int charger_reseau(const char *filename, reseau_t *reseau) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur ouverture fichier");
        return -1;
    }

    // Lecture du nombre d'équipements et de liens
    int nb_equipements, nb_liens;
    if (fscanf(file, "%d %d\n", &nb_equipements, &nb_liens) != 2) {
        fclose(file);
        return -2;
    }
    reseau->nb_equipements = nb_equipements;
    reseau->nb_liens = nb_liens;

    // Lecture des équipements
    char line[256];
    for (int i = 0; i < nb_equipements; i++) {
        if (!fgets(line, sizeof(line), file)) {
            fclose(file);
            return -3;
        }
        int type = -1;
        sscanf(line, "%d;", &type);
        char *ptr = strchr(line, ';');
        if (!ptr) continue;
        ptr++; // après le premier ';'

        // Traitement différent selon le type d'équipement (station ou switch)
        if (type == 1) { // station
            char mac_str[32], ip_str[32];
            sscanf(ptr, "%31[^;];%31[^\n]", mac_str, ip_str);
            reseau->equipements[i].type = STATION;
            reseau->equipements[i].data.station.mac = parse_mac(mac_str);
            reseau->equipements[i].data.station.ip = parse_ip(ip_str);
        } else if (type == 2) { // switch
            char mac_str[32];
            int nb_ports, priority;
            sscanf(ptr, "%31[^;];%d;%d", mac_str, &nb_ports, &priority);
            reseau->equipements[i].type = SWITCH;
            reseau->equipements[i].data.sw.mac = parse_mac(mac_str);
            reseau->equipements[i].data.sw.nb_ports = nb_ports;
            reseau->equipements[i].data.sw.priority = priority;
        } else {
            fclose(file);
            return -4;
        }
    }

    // Lecture des liens entre équipements
    for (int i = 0; i < nb_liens; i++) {
        if (!fgets(line, sizeof(line), file)) {
            fclose(file);
            return -5;
        }
        int e1, e2, poids;
        if (sscanf(line, "%d;%d;%d", &e1, &e2, &poids) != 3) {
            fclose(file);
            return -6;
        }
        reseau->liens[i].equip1 = e1;
        reseau->liens[i].equip2 = e2;
        reseau->liens[i].poids = poids;
    }

    // Initialisation des tables de voisins pour chaque switch
    for (int i = 0; i < reseau->nb_equipements; i++) {
        if (reseau->equipements[i].type == SWITCH) {
            reseau->equipements[i].data.sw.nb_ports = 0;
        }
    }

    // Configuration des ports des switches en fonction des liens
    for (int i = 0; i < reseau->nb_liens; i++) {
        int e1 = reseau->liens[i].equip1;
        int e2 = reseau->liens[i].equip2;

        // Configuration du port pour le premier équipement
        if (reseau->equipements[e1].type == SWITCH) {
            switch_t *sw1 = &reseau->equipements[e1].data.sw;
            sw1->port_table[sw1->nb_ports] = e2;
            // Les ports vers les stations sont toujours actifs, ceux vers les switches sont gérés par STP
            if (reseau->equipements[e2].type == STATION)
                sw1->port_etat[sw1->nb_ports] = 1;
            else
                sw1->port_etat[sw1->nb_ports] = 0;
            sw1->nb_ports++;
        }

        // Configuration du port pour le second équipement
        if (reseau->equipements[e2].type == SWITCH) {
            switch_t *sw2 = &reseau->equipements[e2].data.sw;
            sw2->port_table[sw2->nb_ports] = e1;
            if (reseau->equipements[e1].type == STATION)
                sw2->port_etat[sw2->nb_ports] = 1;
            else
                sw2->port_etat[sw2->nb_ports] = 0;
            sw2->nb_ports++;
        }
    }
    fclose(file);
    return 0;
}
