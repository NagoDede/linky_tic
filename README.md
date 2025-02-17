# linkytic
interface basique pour lire les données tic du compteur linky et les partager en format json via un esp8266/node-mcu.  
## Configuration  
Créer un fichier secret.h (pour définir les logins et pwd du wifi.
'''
// Secrets for your local home network

// This is a "hard way" to configure your local WiFi network name and passphrase
// into the source code and the uploaded sketch.
// 
// Using the WiFi Manager is preferred and avoids reprogramming when your network changes.
// See https://homeding.github.io/#page=/wifimanager.md

// ssid and passPhrase can be used when compiling for a specific environment as a 2. option.

// add you wifi network name and PassPhrase or use WiFi Manager
#ifndef STASSID
#define STASSID "Wifi_Id"
#define STAPSK "Wifi_pwd"
#endif

const char *ssid = STASSID;
const char *passPhrase = STAPSK;
'''

Le code est prévu pour un linky en mode standard. Il devrait être fonctionel en mode historique en changent la vitesse du port (1500bps en historique, 9600bps en standard).  

##Interface
Un simple optocoupleur suffit.
Le schéma disponible sur https://hallard.me/demystifier-la-teleinfo/ est fonctionel chez moi (sans ajout du transistor).

##Documents
Enedis-NOI-CPT_54E version 3 (01/06/2018)




