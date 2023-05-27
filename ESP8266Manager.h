#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <vector>

class Persistence
{
private:
  static const int MAX_KEY_LENGTH = 32;
  static const int MAX_VALUE_LENGTH = 255;

public:
  struct Entry
  {
    char key[MAX_KEY_LENGTH];
    union
    {
      char stringValue[MAX_VALUE_LENGTH];
      byte byteValue;
    };
    bool isString;
  };

  static std::vector<Entry> entries;

  static void addStringData(const char *key, const char *value)
  {
    Entry newEntry;
    strncpy(newEntry.key, key, MAX_KEY_LENGTH);
    strncpy(newEntry.stringValue, value, MAX_VALUE_LENGTH);
    newEntry.isString = true;
    entries.push_back(newEntry);
    saveData();
  }

  static void addByteData(const char *key, byte value)
  {
    Entry newEntry;
    strncpy(newEntry.key, key, MAX_KEY_LENGTH);
    newEntry.byteValue = value;
    newEntry.isString = false;
    entries.push_back(newEntry);
    saveData();
  }

  static const char *loadStringData(const char *key)
  {
    loadData();
    for (const auto &entry : entries)
    {
      if (entry.isString && strcmp(entry.key, key) == 0)
      {
        return entry.stringValue;
      }
    }
    return nullptr;
  }

  static bool loadByteData(const char *key, byte *value)
  {
    loadData();
    for (const auto &entry : entries)
    {
      if (!entry.isString && strcmp(entry.key, key) == 0)
      {
        *value = entry.byteValue;
        return true;
      }
    }
    return false;
  }

private:
  static void saveData()
  {
    EEPROM.begin(MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1 + entries.size());
    for (size_t i = 0; i < entries.size(); ++i)
    {
      const Entry &entry = entries[i];
      EEPROM.put(i * (MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1), entry);
    }
    EEPROM.commit();
    EEPROM.end();
  }

  static void loadData()
  {
    entries.clear();
    EEPROM.begin(MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1);
    for (size_t i = 0; i < EEPROM.length() / (MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1); ++i)
    {
      Entry entry;
      EEPROM.get(i * (MAX_KEY_LENGTH + MAX_VALUE_LENGTH + 1), entry);
      entries.push_back(entry);
    }
    EEPROM.end();
  }
};

std::vector<Persistence::Entry> Persistence::entries;

class OTAManager {
public:
  OTAManager() {

    
    ArduinoOTA.onStart([this]() {
      onStart();
    });
    
    ArduinoOTA.onEnd([this]() {
      onEnd();
    });
    
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
      onProgress(progress, total);
    });
    
    ArduinoOTA.onError([this](ota_error_t error) {
      onError(error);
    });
    
    ArduinoOTA.begin();
  }
  
  void handle() {
    ArduinoOTA.handle();
  }
  
private:
  void onStart() {
    // Callback appelée au début de la mise à jour OTA
    Serial.println("Mise à jour OTA en cours...");
  }
  
  void onEnd() {
    // Callback appelée à la fin de la mise à jour OTA
    Serial.println("\nMise à jour OTA terminée!");
  }
  
  void onProgress(unsigned int progress, unsigned int total) {
    // Callback appelée pendant la mise à jour OTA pour afficher la progression
    Serial.printf("Progression : %u%%\r", (progress / (total / 100)));
  }
  
  void onError(ota_error_t error) {
    // Callback appelée en cas d'erreur lors de la mise à jour OTA
    Serial.printf("Erreur [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Erreur d'authentification");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Erreur de début");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Erreur de connexion");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Erreur de réception");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Erreur de fin");
    }
  }
};
class WiFiManager
{
public:
  WiFiManager() : server(80)
  {

    const char *ssid = Persistence::loadStringData("ssid");
    if (ssid == nullptr)
    {

      Persistence::addStringData("ssid", "ssid");
    }
    const char *password = Persistence::loadStringData("password");
    if (password == nullptr)
    {

      Persistence::addStringData("password", "password");
    }
    const char *hostname = Persistence::loadStringData("hostname");
    if (hostname == nullptr)
    {

      Persistence::addStringData("hostname", "my_esp");
    }

    if (!beginClient(ssid, password, hostname))
    {
      beginAP("my_esp", "12345678");
    }
  }

  void beginAP(const char *ssid, const char *password)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    server.on("/", [this]()
              { handleRoot(); });

    server.on("/config", [this]()
              { handleConfig(); });

    server.begin();
  }

  bool beginClient(const char *ssid, const char *password, const char *hostname)
  {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(hostname); // Définir le hostname
    WiFi.begin(ssid, password);
    int counteur = 0;
    while (!isConnected() || counteur < 10)
    {
      delay(1000);
      counteur++;
    }
    if (!isConnected())
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  bool isConnected()
  {
    return (WiFi.status() == WL_CONNECTED);
  }

  void setHostname(const char *hostname)
  {
    WiFi.hostname(hostname);
  }

  void handle()
  {
    server.handleClient();
    ota.handle();
  }

private:
  ESP8266WebServer server;
  OTAManager ota;
  void handleRoot()
  {
    String html = "<html><body>";
    html += "<h1>Configuration</h1>";
    html += "<form action=\"/config\" method=\"post\">";
    html += "SSID: <input type=\"text\" name=\"ssid\"><br>";
    html += "Password: <input type=\"password\" name=\"password\"><br>";
    html += "Hostname: <input type=\"text\" name=\"hostname\"><br>";
    html += "<input type=\"submit\" value=\"Save\">";
    html += "</form>";
    html += "</body></html>";

    server.send(200, "text/html", html);
  }

  void handleConfig()
  {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String hostname = server.arg("hostname");

    // Enregistrer les paramètres
    saveConfig(ssid, password, hostname);

    String html = "<html><body>";
    html += "<h1>Configuration Saved</h1>";
    html += "<p>Please restart the device.</p>";
    html += "</body></html>";

    server.send(200, "text/html", html);
  }

  void saveConfig(const String &ssid, const String &password, const String &hostname)
  {
    Persistence::addStringData("ssid", ssid.c_str());
    Persistence::addStringData("password", password.c_str());
    Persistence::addStringData("hostname", hostname.c_str());
    WiFi.hostname(hostname.c_str());
  }
};

#endif // WIFI_MANAGER_H
