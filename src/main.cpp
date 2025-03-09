
#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
#include <config.h>
#include "DMA_WiFiManager.h"
#include <time.h>
#include <ESP32_FTPClient.h>
#include <SD.h> 
#include <HTTPClient.h>
#include <Update.h>
const char* ota_url = "https://raw.githubusercontent.com/DataSoft-Manufacturing-and-Assembly/AMR_WITH_DATALOG/main/ota/firmware.bin";
void performOTA();
File logFile;
ESP32_FTPClient ftp(FTP_SERVER, FTP_USER, FTP_PASS, 5000, 2); 

// Function to check if a year is a leap year
bool isLeapYear(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Function to get the number of days in a given month
int daysInMonth(int month, int year) {
  switch (month) {
    case 1:  // February
      return isLeapYear(year) ? 29 : 28;
    case 3: case 5: case 8: case 10:  // April, June, September, November
      return 30;
    default:
      return 31;  // All other months
  }
}

// Function to connect to Wi-Fi and synchronize time with NTP server
bool syncTimeWithNTP() {


  if (WiFi.status() == WL_CONNECTED) {
   
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    if (getLocalTime(&currentTime)) {
      Serial.println("Time synchronized with NTP");

      return true;
    } else {
      Serial.println("Failed to obtain time from NTP");
    }
  } else {
    Serial.println("WiFi connection failed");
  }

  return false;  // Return false if sync was unsuccessful
}


// Function to increment the current time by one second and adjust for date rollovers
void incrementTime() {
  currentTime.tm_sec++;  // Increment seconds

  if (currentTime.tm_sec >= 60) {
    currentTime.tm_sec = 0;
    currentTime.tm_min++;  // Increment minutes

    if (currentTime.tm_min >= 60) {
      currentTime.tm_min = 0;
      currentTime.tm_hour++;  // Increment hours

      if (currentTime.tm_hour >= 24) {
        currentTime.tm_hour = 0;
        currentTime.tm_mday++;  // Increment day

        int daysInCurrentMonth = daysInMonth(currentTime.tm_mon, currentTime.tm_year + 1900);
        
        if (currentTime.tm_mday > daysInCurrentMonth) {
          currentTime.tm_mday = 1;
          currentTime.tm_mon++;  // Increment month

          if (currentTime.tm_mon >= 12) {
            currentTime.tm_mon = 0;
            currentTime.tm_year++;  // Increment year
          }
        }
      }
    }
  }
}






// WIFI and MQTT

// Create a DMA_WiFiManager instance
DMA_WiFiManager WiFiManager("DMA_Wifi_manager", "1234567890");



/************************ FUNCTIONS *********************************/

// LED
void Green();
void Blue();
void Red();
void Cyan();
void Magenta();

// DISPLAY
void display();
LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD address to 0x27 for a 16 chars and 2 line display

WiFiClient espClient;
PubSubClient client(espClient);
Preferences preferences; // Use Preferences library for NVS

// Task handlers
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

// Function declarations
void mqttLoop(void *param);
void mainLoop(void *param);
void timeloop(void *param);
void updater(void *param);




////////######## FTP Data logger  ///////////


void CREAT();
void sendftplog();
void sendftplos();
void write(String payload);
void deleteLogFile();



/////////////////////////
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert payload to String
  String receivedMessage = "";
  for (unsigned int i = 0; i < length; i++) {
    receivedMessage += (char)payload[i];
  }
  Serial.println(receivedMessage);

  if (String(topic) == "DMA/AMR/" + id) {
    if (receivedMessage.startsWith("query:water")) {
      // Respond with current water meter value
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,water:%d", id.c_str(), metter);
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);

      
    } 
    
    if (receivedMessage.startsWith("query:meter no")) {
      // Respond with current water meter value
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,meter no:%s", id.c_str(), Meter_No);
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);

      
    } 



         if (receivedMessage.startsWith("query:heartbeat")) {
      // Respond with current water meter value
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,heartbeat:%d",id.c_str(), PDI);
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
    } 

       if (receivedMessage.startsWith("query:flow")) {
      // Respond with current water meter value
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,flow:%d",id.c_str(), flow);
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
    } 
       if (receivedMessage.startsWith("query:log")) {
        sendftplog();
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s, ftp file send to server",id.c_str());
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
    } 
       if (receivedMessage.startsWith("query:los")) {
        sendftplos();
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s, ftp file send to server",id.c_str());
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
    } 
    if (receivedMessage.startsWith("query:remove")) {
      deleteLogFile();
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,Data loger file removed ",id.c_str());
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
    } 





    



 

///// code for set fucntion 


     if (receivedMessage.startsWith("set:flow=")) {
      String valueString = receivedMessage.substring(9); 
            Serial.print("Extracted flow value string: ");
          Serial.println(valueString);
          int newValue = valueString.toInt();

        if (newValue != 0 || valueString == "0") {
        flow = newValue;

        // Save the new value to NVS
        preferences.begin("storage", false);
        preferences.putInt("flow", flow);
        preferences.end();
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,flow set success",id.c_str());
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
        }
    } 
    if (receivedMessage.startsWith("set:update")) {
      xTaskCreatePinnedToCore(updater, "updater", 10*1024, NULL, 1, &Task3, 1);
    } 
         if (receivedMessage.startsWith("set:heartbeat=")) {
      String valueString = receivedMessage.substring(14); 
            Serial.print("Extracted heartbeat value string: ");
          Serial.println(valueString);
          int newValue = valueString.toInt();

        if (newValue != 0 || valueString == "0") {
        PDI = newValue;

        // Save the new value to NVS
        preferences.begin("storage", false);
        preferences.putInt("PDI", PDI);
        preferences.end();
      char responsePayload[60];
      snprintf(responsePayload, sizeof(responsePayload), "%s,heartbeat set success",id.c_str());
      Serial.println(responsePayload);
      client.publish("DMA/AMR/PUB", responsePayload);
        }
    } 



if (receivedMessage.startsWith("set:meter no=")) {
    // Extract the value string from the message
    String valueString = receivedMessage.substring(13); 
    Serial.print("Extracted meter no value string: ");
    Serial.println(valueString);

    // Assuming Meter_No is a string and not an int
    if (valueString.length() > 0) {
        Meter_No = valueString;

        // Save the new value to NVS
        preferences.begin("storage", false);
        preferences.putString("Meter_No", Meter_No);  // Save as a string
        preferences.end();
        
        Serial.print("Meter number updated to: ");
        Serial.println(Meter_No);
        char responsePayload[60];
         snprintf(responsePayload, sizeof(responsePayload), "%s,meter no set success",id.c_str());
         Serial.println(responsePayload);
        client.publish("DMA/AMR/PUB", responsePayload);


         
    } else {
        Serial.println("Invalid meter number.");
    }
  }
    

    else if (receivedMessage.startsWith("set:water=")) {
      // Extract the value after "set:water="
      String valueString = receivedMessage.substring(10); // Get value after "set:water="
      
      // Debug print to check the substring
      Serial.print("Extracted value string: ");
      Serial.println(valueString);

      // Convert the extracted string to integer
      int newValue = valueString.toInt(); // Convert to integer
      Serial.print("Parsed integer value: ");
      Serial.println(newValue);

      // Check if the conversion was successful
      if (newValue != 0 || valueString == "0") {
        metter = newValue;

        // Save the new value to NVS
        preferences.begin("storage", false);
        preferences.putInt("metter", metter);
        preferences.end();

        char responsePayload[60];
         snprintf(responsePayload, sizeof(responsePayload), "%s,water set success",id.c_str());
         Serial.println(responsePayload);
        client.publish("DMA/AMR/PUB", responsePayload);
       
      } else {
        Serial.println("Error: Invalid water meter value received.");
      }
    }
  }
}


void reconnect() {
    if (WiFi.status() == WL_CONNECTED) {


      if (!client.connected()) {
          Serial.print("Attempting MQTT connection...");
          String clientId = "ESP32Client-";
          clientId += String(random(0xffff), HEX);

      if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
          char payload[45];
          snprintf(payload, sizeof(payload), "%s,wifi connected", id.c_str());
          client.publish("DMA/AMR/PUB", payload);
          Green();
          Serial.println("connected");
        if( sd_state==true ){
              if (syncTimeWithNTP()) {             // Initial sync with NTP if possible
                    lastSyncTime = millis();           // Record the successful sync time
                } 
            else {
                Serial.println("Starting without ntp sync  ");
              }
        }
      
      Green();

      String topic = "DMA/AMR/" + id;
      client.subscribe(topic.c_str());
    } 
    else {
      Red();
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      Serial.println("failed, try again in 5 seconds");
      delay(5000);
    }
  }



  }
  else{
    Cyan();
    Serial.println("wifi error");
    delay(2000);
    reset_count++;
    if(reset_count>limiter){
      reset_count=0;
    // ESP.restart();
      WiFiManager.begin();

    }
  }

}

void Green() {
    leds[0] = CRGB::Black;
  FastLED.show();
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}


void Cyan() {
    leds[0] = CRGB::Black;
  FastLED.show();
  leds[0] = CRGB::Cyan;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}

void Red() {
    leds[0] = CRGB::Black;
  FastLED.show();
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}
void Blue() {
  leds[0] = CRGB::Black;
  FastLED.show();
  leds[0] = CRGB::Blue;
  FastLED.show();
  delay(500);
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(500);
}

void Magenta() {
  leds[0] = CRGB::Magenta;
  FastLED.show();

}

void display() {
  value = (float)metter / 1000; 
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AMR v2.1");
  lcd.setCursor(0, 1);
  lcd.print("Developed by DMA");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  DISPLAY_UP = prefixUP + Meter_No;
  lcd.print(DISPLAY_UP);
  lcd.setCursor(0, 1);
  String valueStr = String(value, 2);
  DISPLAY_DOWN = prefix + valueStr + suffix;
  lcd.print(DISPLAY_DOWN);
  delay(4000);
}

// Task for handling MQTT and WiFi
void mqttLoop(void *param) {
    WiFiManager.begin();
  for (;;) {
    WiFiManager.handleClient(); 
    if (!client.connected()) {
        reconnect();
      }
    client.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);  //jate com cpu use hoy

  }
}




// Task for handling time and filesystem type thisngs



void updater(void *param) {
       performOTA();
      vTaskDelay(10 / portTICK_PERIOD_MS);
}






















void timeloop(void *param) {

  Serial.println("time initialize");
  for (;;) {

  if( sd_state==true){
      incrementTime();
      if (millis() - lastSyncTime >= syncInterval) {
            if (syncTimeWithNTP()) {
                 lastSyncTime = millis();         // Update last sync time if successful
              }
        }
    if(signalstate==1){
      char payload[60];
      snprintf(payload, sizeof(payload), "%s,C:%d,S:0,TW:%d,F:100,connected", id.c_str(), CHARGING_STATE, metter);
      signalstate=0;
      write(payload); 
    }
    delay(1000);
    
    
    }
    else{
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    

  }

}




// Main task
void mainLoop(void *param) {
  for (;;) {


   // Serial.println("Ok");  // Print "Ok" every second to confirm main loop is working
    int pinState = digitalRead(buttonPin);
    //Serial.println(pinState);

    CHARGING_STATE = digitalRead(CHARGING_PIN);

    int LED = digitalRead(14);
    if (LED == 0) {
      backlightTime = millis();
    }
    if (millis() - backlightTime <= 5000) {
      display();
    } else {
      lcd.noBacklight();
    }






  static unsigned long pressStartTime = 0; // To track when the button was pressed
  static bool isPressed = false;          // To track if the button is being pressed


  if (digitalRead(14) == 0) {
    Serial.println("press");
    if (!isPressed) { // Button was just pressed
      isPressed = true;
      pressStartTime = millis(); // Record the time the button was pressed
    } else if (millis() - pressStartTime >= 5000) {
            Magenta();
           
        Serial.println("Button 14 triggering reconfiguration");

           
            WiFiManager.startAP();
             //   Serial.println("2nd file");
             //ESP.restart();
      isPressed = false; // Reset the press state
    }
  } else {
    // Button is not pressed
    isPressed = false; // Reset the press state
  }





    //Serial.println(pinState);
    // Button logic to increment the meter value
    if (point_signal == 1) {
      if (holdTime == 0) {
        holdTime = millis();
      }

      if (pinState == LOW) {
        if (millis() - holdTime >= releaseDuration) {
          point_signal = 0;
          holdTime = 0;
          Serial.println("ready");
        }
      } else {
        point_signal = 1;
        holdTime = 0;
      }
    }

    if (point_signal == 0) {
      if (pinState == HIGH) {
        if (startTime == 0) {
          startTime = millis();
        } else if (millis() - startTime >= holdDuration) {


          Serial.println("Published: ok");
          metter += flow;  // Increment metter value
          signalstate=1;
          Blue();
          preferences.begin("storage", false);  // Save new value to NVS
          preferences.putInt("metter", metter);
          preferences.end();
          value = (float)metter / 1000;
          lcd.clear();
          String valueStr = String(value, 2);
          DISPLAY_DOWN = prefix + valueStr + suffix;
          lcd.print(DISPLAY_DOWN);
          startTime = 0;
          point_signal = 1;
        }
      } else {
        startTime = 0;
      }
    }
    long int temptime;
    // Sending data packet every interval
    if (temptime == 0) {
      temptime = millis();
    }
    if (millis() - temptime > PDI) {
      char payload[60];
      snprintf(payload, sizeof(payload), "%s,C:%d,S:0,TW:%d,F:100,connected", id.c_str(), CHARGING_STATE, metter);
      Serial.print("Uploading data... ");

      if (WiFi.status() == WL_CONNECTED){
        client.publish("DMA/AMR/PUB", payload);
        Serial.println("success");
      }
      else{
        Serial.println("failed to upload data....");
      }


      temptime = 0;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);  //jate com cpu use hoy
  }
}

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  pinMode(buttonPin, INPUT);
  pinMode(CHARGING_PIN, INPUT);
  Serial.begin(115200);

  #if FFS_CREAT == 1
  preferences.begin("storage", false);
  preferences.putString("id", id);
  preferences.end();
  #endif
  //pinMode(2,OUTPUT);
  Serial.print("device id: ");
  Serial.println(id);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  preferences.begin("storage", false);
  metter = preferences.getInt("metter", 0);
  flow = preferences.getInt("flow", 100);
  PDI = preferences.getInt("PDI", 300000);
  Meter_No = preferences.getInt("Meter_No", 0000);
  id = preferences.getString("id", id);
  preferences.end();


  // Initialize the LCD display
  lcd.init();
  display();

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    sd_state=false;

    
  }
  else{
    Serial.println("SD card initialized");
    sd_state=true;
    CREAT();
    xTaskCreatePinnedToCore(timeloop, "time Task", 10*1024, NULL, 1, &Task3, 0);
    }
  if (syncTimeWithNTP()) {             // Initial sync with NTP if possible
    lastSyncTime = millis();           // Record the successful sync time
  } else {
    Serial.println("Starting without NTP sync");
  }
  
  int pinState = digitalRead(buttonPin);
   if (pinState == HIGH) {
        point_signal = 1;

   }
  

  xTaskCreatePinnedToCore(mqttLoop, "MQTT Task", 10*1024, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(mainLoop, "Main Task", 10*1024, NULL, 1, &Task2, 1);

}

void loop() {
  // kill me now
}




////////////////////////// this part of code is responcible to write inside the filesystem
void write(String payload) {
  Serial.println("in write function ");
  String timeString = 
                    (currentTime.tm_hour < 10 ? "0" : "") + String(currentTime.tm_hour) + ":" +
                    (currentTime.tm_min < 10 ? "0" : "") + String(currentTime.tm_min) + " " + 
                    (currentTime.tm_mon + 1 < 10 ? "0" : "") + String(currentTime.tm_mon + 1) + "/" +
                    (currentTime.tm_mday < 10 ? "0" : "") + String(currentTime.tm_mday) + "/" + 
                    String(currentTime.tm_year + 1900)+",";

// Append ".csv" to the time string
timeString += payload;
Serial.println("time created");



  logFile = SD.open("/log.csv", FILE_APPEND);
  
  if (logFile) {
    logFile.println(timeString);  // Write the data to the file
    logFile.close();        // Close the file
    Serial.println("Data written to log.csv: " + timeString);
  } else {
    Serial.println("Failed to open log.csv for writing");
  }

    if (WiFi.status() != WL_CONNECTED || !client.connected() )  {
    File file = SD.open("/los.csv", FILE_APPEND);
    if (file) {
      
      file.println(timeString); // Log the input from the serial monitor
      file.close();
      Serial.print("Logged in /los.csv: ");

    } else {
      Serial.println("Error opening /los.csv for writing.");
    }
  }







}


void CREAT() {
  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Check if the /log.csv file exists, if not create it
  if (!SD.exists("/log.csv")) {
    File file = SD.open("/log.csv", FILE_WRITE);
    if (file) {
      //file.println("Press Count, Status"); // Add header to CSV
      file.close();
      Serial.println("/log.csv file created.");
    } else {
      Serial.println("Failed to create /log.csv file!");
    }
  }

    // Check if the /los.csv file exists, if not create it
  if (!SD.exists("/los.csv")) {
    File file = SD.open("/los.csv", FILE_WRITE);
    if (file) {
      //file.println("Press Count, Status"); // Add header to CSV
      file.close();
      Serial.println("/los.csv file created.");
    } else {
      Serial.println("Failed to create /los.csv file!");
    }
  }

}


void sendftplog(){

  String timeString = 
                    (currentTime.tm_hour < 10 ? "0" : "") + String(currentTime.tm_hour) + "_" +
                    (currentTime.tm_min < 10 ? "0" : "") + String(currentTime.tm_min) + " " + 
                    (currentTime.tm_mon + 1 < 10 ? "0" : "") + String(currentTime.tm_mon + 1) + "_" +
                    (currentTime.tm_mday < 10 ? "0" : "") + String(currentTime.tm_mday) + "_" + 
                    String(currentTime.tm_year + 1900);

// Append ".csv" to the time string
timeString += " ";
timeString += id;
timeString += " log";
timeString += ".csv";

// Convert String to const char*
const char* timeChar = timeString.c_str();

  // Connect to FTP server
  ftp.OpenConnection();
  Serial.println("Connected to FTP server");

  // Change working directory on the FTP server
  ftp.ChangeWorkDir("/home/dmacam/intrusion/AMR");
  Serial.println("Directory changed");

  // Open the file from SD card
  File file = SD.open("/log.csv");
  if (!file) {
    Serial.println("Failed to open los.csv on SD card");
    ftp.CloseConnection();
    return;
  }

  ftp.InitFile("Type A");
  ftp.NewFile(timeChar);
  byte buffer[128];  //in this part it will read the file bit my bit then upload it
  while (file.available()) {
    int bytesRead = file.read(buffer, sizeof(buffer));
    ftp.WriteData(buffer, bytesRead);  // Write data to FTP server
  }
  Serial.println("File uploaded successfully");
  ftp.CloseFile();  // Close the file transfer
  file.close();
  ftp.CloseConnection();

}


void sendftplos(){

  String timeString = 
                    (currentTime.tm_hour < 10 ? "0" : "") + String(currentTime.tm_hour) + "_" +
                    (currentTime.tm_min < 10 ? "0" : "") + String(currentTime.tm_min) + " " + 
                    (currentTime.tm_mon + 1 < 10 ? "0" : "") + String(currentTime.tm_mon + 1) + "_" +
                    (currentTime.tm_mday < 10 ? "0" : "") + String(currentTime.tm_mday) + "_" + 
                    String(currentTime.tm_year + 1900);

// Append ".csv" to the time string
timeString += " ";
timeString += id;
timeString += " los";
timeString += ".csv";

// Convert String to const char*
const char* timeChar = timeString.c_str();

  // Connect to FTP server
  ftp.OpenConnection();
  Serial.println("Connected to FTP server");

  // Change working directory on the FTP server
  ftp.ChangeWorkDir("/home/dmacam/intrusion/AMR");
  Serial.println("Directory changed");

  // Open the file from SD card
  File file = SD.open("/los.csv");
  if (!file) {
    Serial.println("Failed to open los.csv on SD card");
    ftp.CloseConnection();
    return;
  }

  ftp.InitFile("Type A");
  ftp.NewFile(timeChar);
  byte buffer[128];  //in this part it will read the file bit my bit then upload it
  while (file.available()) {
    int bytesRead = file.read(buffer, sizeof(buffer));
    ftp.WriteData(buffer, bytesRead);  // Write data to FTP server
  }
  Serial.println("File uploaded successfully");
  ftp.CloseFile();  // Close the file transfer
  file.close();
  ftp.CloseConnection();

}


void deleteLogFile() {
  // Check if the file exists
  if (SD.exists("/log.csv")) {
    // Try to remove the file
    if (SD.remove("/log.csv")) {
      Serial.println("log.csv has been deleted successfully.");
    } else {
      Serial.println("Failed to delete log.csv.");
    }
  } else {
    Serial.println("log.csv does not exist.");
  }
  if (SD.exists("/los.csv")) {
    // Try to remove the file
    if (SD.remove("/los.csv")) {
      Serial.println("los.csv has been deleted successfully.");
    } else {
      Serial.println("Failed to delete los.csv.");
    }
  } else {
    Serial.println("los.csv does not exist.");
  }
}

void performOTA() {
  Serial.println("Starting OTA update...");

  HTTPClient http;
  http.begin(ota_url);

  int httpCode = http.GET();  // Send HTTP GET request
  if (httpCode == HTTP_CODE_OK) {
      int contentLength = http.getSize();
      Serial.printf("Content-Length: %d bytes\n", contentLength);

      if (contentLength > 0 && Update.begin(contentLength)) {
          WiFiClient* stream = http.getStreamPtr();
          size_t written = Update.writeStream(*stream);

          if (written == contentLength && Update.end() && Update.isFinished()) {
              Serial.println("OTA update completed. Restarting...");
              delay(1000);
              ESP.restart();  // Restart ESP32 after successful update
          } else {
              Serial.println("OTA update failed!");
          }
      } else {
          Serial.println("OTA begin failed!");
      }
  } else {
      Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}











