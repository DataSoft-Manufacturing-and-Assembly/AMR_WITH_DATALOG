/*#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
*/
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

extern int closewifi;
class DMA_WiFiManager {
public:
    DMA_WiFiManager(const char* apSSID, const char* apPassword);
    void begin();
    void handleClient();
    void connect();
    void startAP();
    bool isConnected();

private:
int count=0;
    Preferences preferences;
    WebServer server;
    const char* ssid;
    const char* password;
    String ssidArray[6];
    String name[3];
    String origin[3];
    String arraySSIDCHECK[6];
    String arrayPASSCHECK[6];
    int setCount;
    
    const int maxSSIDs = 6;  // Define the maximum number of SSIDs to store
    unsigned long pressStartTime;
    unsigned long previousMillis = 0; // stores the last time a message was printed
    int interval = 20000; // interval for 5 seconds
    int initial = 0;
    unsigned long currentMillis=0;
    //int debug=0;

    String generateHTML();
    void setupRoutes();
};

//#endif
