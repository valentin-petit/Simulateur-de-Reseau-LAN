#ifndef COMMUTATION_H
#define COMMUTATION_H

#include "equipement.h"

// Ajoute une MAC à la table si pas déjà là, ou met à jour le port si besoin
int switch_apprendre_mac(switch_t *sw, mac_addr_t mac_src, int port);

// Cherche le port associé à une MAC, -1 si inconnu
int switch_rechercher_port(switch_t *sw, mac_addr_t mac_dest);

// Affiche la table MAC d'un switch
void afficher_table_mac(switch_t *sw);

#endif 