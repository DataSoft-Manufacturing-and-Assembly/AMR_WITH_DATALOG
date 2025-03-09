


/*******************************WIFI_MQTT_CONFIG********************************/


const char* mqtt_server = "broker2.dma-bd.com";  // New server
const char* mqtt_user = "broker2";               // New username
 const char* mqtt_password = "Secret!@#$1234";    // New password
//const char* mqtt_server = "192.168.1.110";  // New server
//const char* mqtt_user = "";               // New username
//const char* mqtt_password = "";    // New password
/*******************************************************************************

/*******************************DEVICE_ID_CONFIG********************************/

#define DEVICE_SERIAL_ID                             "0000"//63
#define DEVICE_CODE_UPLOAD_DATE                      "240909"
#define WORK_PACKAGE                                 "114600"
String id =  String(WORK_PACKAGE) + String(DEVICE_CODE_UPLOAD_DATE) +  String(DEVICE_SERIAL_ID) ;
String  Meter_No= "0000";

#define FFS_CREAT 1
/*******************************************************************************/




/*******************************SENSOR_CONFIG********************************/
int point_signal = 0;
int metter = 0; // Your integer value
const int buttonPin = 39; // Pin to monitor
// Variables to handle timing
unsigned long startTime = 0;
unsigned long holdTime = 0;
const unsigned long holdDuration = 4000; // 4 seconds
const unsigned long releaseDuration = 4000; // 4 seconds
int flow = 100;
/*******************************************************************************/



/*******************************LCD_CONFIGARATION********************************/
String prefix = "TW: "; 
String prefixUP = "MN: ";   // First string
float value = 00.0;     // Float value
String suffix = "m3";
String DISPLAY_DOWN; 
String DISPLAY_UP; 
unsigned long backlightTime = 0;
/***************************************************************************/


/************************PACKAGE_SEND_INTERVAL***************************/


int PDI = 300000 ; //time interval of sending a package
/***************************************************************************/



/************************ON BOARD LED*********************************/
#define NUM_LEDS 1
#define DATA_PIN 4
#define CLOCK_PIN 13
CRGB leds[NUM_LEDS];
/***************************************************************************/


/************************CHARGING_STATE*********************************/
int CHARGING_PIN=34;
int CHARGING_STATE=0;
/***********************************************************************/
int signalstate = 0;
int reset_count=0; 
int limiter=30; 



 

///************** FTP and NTP ************** */



const int chipSelect = 5;  // SD card CS pin
String timeString = "15_25 09_11_2`025.csv";
#define FTP_SERVER "iot2.dma-bd.com"
#define FTP_USER "dmacam"
#define FTP_PASS "dmabd987!@#$"
#define SD_CS_PIN 5  // Chip select pin for SD card (adjust if necessary)





// Define NTP server and timezone settings for Dhaka, Bangladesh
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 6 * 3600;  // UTC+6 for Dhaka
const int daylightOffset_sec = 0;     // No daylight saving time

struct tm currentTime;                 // Structure to store the current time
unsigned long lastSyncTime = 0;        // Last sync time (in milliseconds)
const unsigned long syncInterval = 60*60*1000; // .5-hour interval in milliseconds
bool sd_state= true;
//#define SD_Card false