# Simulateur de Réseau avec STP (Spanning Tree Protocol)

Un outil académique de simulation réseau LAN, développé en C, conçu pour modéliser le comportement de base des réseaux locaux et analyser l'interaction des protocoles Ethernet et STP (projet non finalisé).

## À propos du projet

Ce simulateur de réseau permet de modéliser et de simuler le comportement d'un réseau local (LAN) composé de :

* **Stations** : Équipements terminaux avec adresses MAC et IP.
* **Commutateurs (Switches)** : Équipements de commutation avec tables MAC, gestion des ports et support STP.


## Fonctionnalités Clés

### Protocole STP (Spanning Tree Protocol)

L'objectif principal est de prévenir les boucles réseau en implémentant l'algorithme STP :

* Calcul automatique de l'arbre couvrant.
* Élection du Root Bridge basée sur la priorité des commutateurs.
* Détermination et gestion des états des ports : Port Racine, Port Désigné, et ports Bloqués.

### Commutation Ethernet

* Apprentissage automatique des adresses MAC et gestion des tables de commutation dynamiques.
* Propagation des Trames Ethernet simulées.

### Simulation et Analyse

Le simulateur propose un menu interactif en ligne de commande pour :

1.  Lancer une simulation de trame (affichage détaillé du chemin et de l'apprentissage MAC).
2.  Afficher l'état des ports STP (actif/bloqué).
3.  Afficher les tables MAC de tous les commutateurs.
4.  Afficher la matrice d'adjacence (topologie du réseau).

## Technologies utilisées

* **Langage de programmation :** C
* **Compilation :** GCC avec flags stricts (`-Wall -Wextra -Werror`), géré par un `Makefile`.  
* **Affichage :** Interface en couleur avec codes ANSI.


## Installation et Exécution
Pour exécuter et contribuer au projet localement, vous devez disposer d'un environnement de compilation C. Ensuite, suivez ces étapes :

1. Cloner votre fork localement : `https://github.com/valentin-petit/Simulateur-de-Reseau-LAN.git`
2. Compiler le projet : make    
3. Lancer le simulateur en spécifiant le fichier de configuration : ./bin/simulateur_reseau reseau_config.txt ou make run  

## Équipe

Ce projet a été réalisé avec :
* **Yvan DUBLANC-SOUBIGOU** 
* **Yannis SALMANE**

## Licence

Ce projet est distribué sous la Licence MIT.
