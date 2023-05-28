# WiFi Manager

Ce projet contient les classes `WiFiManager` et `Persistence` qui facilitent la gestion de la connexion WiFi et de la persistance des données sur un ESP8266.

## Description

La classe `WiFiManager` permet de gérer la connexion WiFi de l'ESP8266. Elle offre les fonctionnalités suivantes :

- Configuration initiale de la connexion WiFi en mode client ou en point d'accès.
- Gestion de la page de configuration via un serveur web embarqué.
- Support de la mise à jour du firmware via OTA (Over-the-Air).

La classe `Persistence` facilite la persistance des données sur l'ESP8266 en utilisant la mémoire EEPROM. Elle permet de stocker et de récupérer des données de type chaîne de caractères (`const char*`) et de type byte (`byte`).

## Utilisation

1. Inclure les bibliothèques nécessaires : `ESP8266WiFi`, `ESP8266WebServer`, `ESP8266HTTPUpdateServer`, `ArduinoOTA`, `EEPROM`.
2. Copier les fichiers `WiFiManager.h` et `WiFiManager.cpp` dans votre projet.
3. Inclure le fichier `WiFiManager.h` dans votre code Arduino : `#include "WiFiManager.h"`.
4. Créer une instance de la classe `WiFiManager`.
5. Appeler la méthode `handle()` de l'instance de `WiFiManager` dans la boucle `loop()` de votre code.

Exemple d'utilisation :

```cpp
#include <Arduino.h>
#include "WiFiManager.h"

WiFiManager wifiManager;

void setup()
{
  Serial.begin(115200);
 
}

void loop()
{
  wifiManager.handle(); // Gérer les requêtes client et OTA
}
Configuration
La configuration initiale peut être effectuée en accédant à la page de configuration du WiFi Manager via l'adresse IP du point d'accès ou en vous connectant à l'ESP8266 en tant que client. La page de configuration affiche un formulaire où vous pouvez saisir les informations de connexion WiFi, telles que le SSID, le mot de passe et le nom d'hôte.

Une fois la configuration enregistrée, vous devrez redémarrer l'appareil pour appliquer les modifications.

; PlatformIO Project Configuration File

[env:esp12e]
platform = espressif8266
board = esp12e
framework = arduino
monitor_speed = 115200

lib_deps =
  ESP8266WiFi
  ESP8266WebServer
  ESP8266HTTPUpdateServer
  ArduinoOTA

upload_port = /dev/ttyUSB0

;pour la mise a jour ota:
upload_port = #put hostname here
upload_protocol = espota