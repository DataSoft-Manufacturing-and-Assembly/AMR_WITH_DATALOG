#include "DMA_WiFiManager.h"

int closewifi=1;
DMA_WiFiManager::DMA_WiFiManager(const char* apSSID, const char* apPassword)
    : server(80), ssid(apSSID), password(apPassword), setCount(0), pressStartTime(0) {}
String DMA_WiFiManager::generateHTML() {
    String html = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32 Array Webpage</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #e0f7fa; /* Light aqua background */
                color: #00796b; /* Darker aqua text */
                text-align: center;
                margin: 0;
                padding: 0;
            }
            h1 {
                background-color: #004d40;
                color: white;
                padding: 20px;
                margin: 0;
            }
            h2 {
                color: #00695c;
            }
            ul {
                list-style-type: none;
                padding: 0;
                margin: 20px 0;
            }
            li {
                cursor: pointer;
                padding: 10px;
                margin: 5px 0;
                background-color: #80deea;
                border-radius: 5px;
                transition: background-color 0.3s ease;
            }
            li:hover {
                background-color: #4db6ac;
            }
            input[type="text"] {
                padding: 8px;
                border: 1px solid #00796b;
                border-radius: 4px;
                width: 80%;
                max-width: 300px;
            }
            button {
                background-color: #00796b;
                color: white;
                padding: 10px 20px;
                border: none;
                border-radius: 4px;
                cursor: pointer;
                font-size: 16px;
                margin: 10px 5px;
                transition: background-color 0.3s ease;
            }
            button:hover {
                background-color: #004d40;
            }
        </style>
    </head>
    <body>
        <h1>DMA WiFi Manager</h1>
        <h2>Available Wifi:</h2>
        <ul>
    )=====";

    for (int i = 0; i < maxSSIDs; i++) {
        html += "<li onclick=\"selectItem('" + ssidArray[i] + "')\">" + ssidArray[i] + "</li>";
    }

    html += R"=====(
        </ul>

        <h3>Enter Details</h3>
        <label>Selected SSID:</label><br>
        <input type="text" id="text1" readonly><br><br>
        <label>PASS:</label><br>
        <input type="text" id="text2"><br><br>

        <button onclick="setValues()">Set</button>
        <button onclick="startFunction()">Start</button>

        <script>
            function selectItem(item) {
                document.getElementById("text1").value = item;
            }

            function setValues() {
                const item = document.getElementById("text1").value;
                const origin = document.getElementById("text2").value;
                fetch(`/set?name=${item}&origin=${origin}`)
                    .then(response => response.text())
                    .then(data => alert(data));
            }

            function startFunction() {
                fetch('/start');
            }
        </script>
    </body>
    </html>
    )=====";

    return html;
}



void DMA_WiFiManager::begin() {
    preferences.begin("wifistore", false);
    bool allEmpty = true;
   // WiFi.mode(WIFI_STA);
    delay(100);


    for (int i = 0; i < 6; i++) {
        String key = "name" + String(i);
        String loadedName = preferences.getString(key.c_str(), "");
        if (!loadedName.isEmpty()) {
            allEmpty = false;
        }
    }

    if (allEmpty) {
        Serial.println("No data found. Starting AP mode.");
        startAP();
    } else {
        Serial.println("Data found. Connecting to WiFi.");
        connect();
    }

    preferences.end();
}

void DMA_WiFiManager::handleClient() {
    server.handleClient();
}









void DMA_WiFiManager::connect() {
 Serial.println("up.");

preferences.begin("wifistore", false);

currentMillis = millis();
//Serial.println("will wifi get conected:");
//Serial.println(initial);

for(int p=0;p<=0;p++) {
    Serial.println("in loop");


int count=0;
int n = WiFi.scanNetworks();
   

    String bestSSID = "";
    int bestRSSI = -100; // Start with a low RSSI
    String bestPass="";
    for (int i = 0; i <n ; ++i) {
      String currentSSID = WiFi.SSID(i);
      int currentRSSI = WiFi.RSSI(i);

      for (int j = 0; j < 5; ++j) {
        String key = "name" + String(j);  // Generate key name
        String loadedName = preferences.getString(key.c_str(), "");
        String key2 = "origin" + String(j);  // Generate key name
        String loadedPass = preferences.getString(key2.c_str(), "");
        if (currentSSID == loadedName) {
          Serial.print("Found SSID: ");
          Serial.print(currentSSID);
          arraySSIDCHECK[count] = currentSSID;
          Serial.print(" with RSSI: ");
          Serial.println(currentRSSI);     
          arrayPASSCHECK[count] = loadedPass;
          count++;
        }
       

      }
    }

    Serial.println("total number of ssid found");
    Serial.println(count);
 
  Serial.println("test on going");
    delay(500);
      for (int i = 0; i < count; i++) {
    Serial.print("SSID: ");
    Serial.print(arraySSIDCHECK[i]);
    Serial.print(" PASS: ");
    Serial.print(arrayPASSCHECK[i]);
    Serial.println();
    WiFi.begin(arraySSIDCHECK[i].c_str(), arrayPASSCHECK[i].c_str());
     int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print("#");
        attempts++;

    }
    if (WiFi.status() == WL_CONNECTED)
    {

      Serial.println("initially connected");
          WiFi.softAPdisconnect(true);   // Stop Soft AP and disconnect clients
        Serial.println("Soft AP stopped.");
         Serial.println("down.");

        preferences.end();
        break;




    }
    else{
        Serial.println("\n SSID or PASS error \n shifting to next ssid");
       

    }

  }
  if (WiFi.status() == WL_CONNECTED)
    {

        break;

    }


  }
  //## main loop end here
   if (WiFi.status() != WL_CONNECTED)
    {

   WiFi.mode(WIFI_OFF);
   //begin();
    }





    




}

void DMA_WiFiManager::startAP() {
    delay(5000);
   
   preferences.begin("wifistore", false);
    preferences.clear(); // Clear preferences in "wifistore"
    Serial.println("Preferences cleared.");

    //WiFi.mode(WIFI_STA);
   // delay(100);

    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    Serial.print("Found networks: ");
    Serial.println(n);

    if (n == 0) {
        Serial.println("No networks found.");
    } else {
        for (int i = 0; i < n; ++i) {
            String currentSSID = WiFi.SSID(i);
            int currentRSSI = WiFi.RSSI(i);

            Serial.print("SSID: ");
            Serial.print(currentSSID);
            Serial.print(" | Signal strength (RSSI): ");
            Serial.println(currentRSSI);

            if (i < maxSSIDs) {
                ssidArray[i] = currentSSID; // Save SSID to the array
            }

            if (i == maxSSIDs - 1) {
                break; // Stop after storing the maximum allowed SSIDs
            }
        }
    }

    WiFi.softAP(ssid, password);
    Serial.println("Access point started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, [this]() {
        server.send(200, "text/html", generateHTML());
    });


  server.on("/set", HTTP_GET, [this]() {
    preferences.begin("wifistore", false); // Open preferences in "wifistore" namespace

    // Ensure both 'name' and 'origin' parameters are provided
    if (server.hasArg("name") && server.hasArg("origin")) {
        String nameParam = server.arg("name");
        String originParam = server.arg("origin");
        //Serial.print(nameParam);
        if (setCount < maxSSIDs) {
            // Construct keys for storing in preferences
            String keyName = "name" + String(setCount);  // Key for SSID
            String keyOrigin = "origin" + String(setCount);  // Key for password

            // Store SSID and password in preferences
            preferences.putString(keyName.c_str(), nameParam);
            preferences.putString(keyOrigin.c_str(), originParam);

            // Debugging: Confirm the values are stored
            String loadedName = preferences.getString(keyName.c_str(), "");
            String loadedOrigin = preferences.getString(keyOrigin.c_str(), "");

            Serial.print("Stored SSID: ");
            Serial.println(loadedName);
            Serial.print("Stored Password: ");
            Serial.println(loadedOrigin);

            // Increment the set count
            setCount++;

            server.send(200, "text/plain", "Values set successfully");
        } else {
            server.send(200, "text/plain", "Maximum sets reached");
        }
    } else {
        server.send(400, "text/plain", "Missing name or origin parameter");
    }

    preferences.end(); // Close preferences
});



    server.on("/start", HTTP_GET, [this]() {
        server.send(200, "text/plain", "Device Connected Closing Wifi HotSpot");
        connect();

    });

    preferences.end();
    server.begin();
}

bool DMA_WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

