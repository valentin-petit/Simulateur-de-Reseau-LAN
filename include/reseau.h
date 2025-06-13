#ifndef RESEAU_H
#define RESEAU_H

#include "equipement.h"

// Charge un réseau à partir d'un fichier de configuration
int charger_reseau(const char *filename, reseau_t *reseau);

#endif