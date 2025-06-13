#ifndef PORT_H
#define PORT_H

#include "equipement.h"

typedef enum {
    PORT_DISCONNECTED = 0,
    PORT_CONNECTED = 1
} PortStatus;

typedef struct {
    int id;                     // Identifiant unique du port
    PortStatus status;          // État du port (connecté/déconnecté)
    int connected_to_equip;     // Index de l'équipement connecté
    int connected_to_port;      // Numéro du port connecté sur l'autre équipement
    mac_addr_t last_src_mac;    // Dernière adresse MAC source vue sur ce port
} Port;

// Structure pour gérer les ports d'un équipement
typedef struct {
    Port ports[MAX_PORTS];
    int nb_ports;
} PortManager;

// Fonctions de gestion des ports
void initialiser_port(Port *port, int id);
void connecter_ports(Port *port1, int equip1, Port *port2, int equip2);
void deconnecter_port(Port *port);
int trouver_port_par_mac(const PortManager *pm, const mac_addr_t *mac);
void mettre_a_jour_table_commutation(switch_t *sw, const mac_addr_t *mac, int port_id);

#endif 
