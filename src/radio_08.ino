#include <Arduino.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>  // lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <Adafruit_VS1053.h>    // don't forget to connect miso/mosi/sck
#define BREAKOUT_CS     5       // VS1053 chip select pin (output) = Control Chip Select Pin (for accessing SPI Control/Status registers) --> XCS
#define BREAKOUT_DCS    33      // VS1053 Data/chip/command select pin (output) --> XDSV sau XDCS
#define DREQ            35      // VS1053 Data request, ideally an Interrupt pin: Player asks for more  data, --> DREQ
#define BREAKOUT_RESET  -1      // VS1053 reset pin (output) --> XRET ; not necessary with esp32, wired to EN pin on ESP32
#define CARDCS          -1      // SD Card command select pin (output); not used
Adafruit_VS1053_FilePlayer vsPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
WiFiClient       netClient;

// global vars; will be overwritten after json parsing
char wifi0_ssid[20], wifi0_pass[20], wifi1_ssid[20], wifi1_pass[20], wifi2_ssid[20], wifi2_pass[20];
char user_agent[20];
char ap_pass[20];
byte wifi_channel = 10;
byte VS_vol_l = 50;
byte VS_vol_r = 50;
byte ST_AMPLITUDE = 15;
byte ST_FREQLIMIT = 8;
byte SB_AMPLITUDE = 15;
byte SB_FREQLIMIT = 15;

#define BUTTON1 32
#define BUTTON2 27
#define BUTTON3 14
#define BUTTON4 12
byte keyLast  = 0;  // contains the last pressed key, even if meanwhile it was depressed; TO DO: read in parallel mode
byte keyReady = 1;  // flag meaning we're ready to process a keystroke

#define RELAYPIN 15  // pin for relay command (pull up pin)

byte fgApp = 0;       // foreground app to run (0 = clock, 1 = internet radio, 2 = station list, 3 = setup)
byte fgAppMax = 4;    // total number of apps + 1
byte fgAppPrev = 100; // previous app

IPAddress local_IP(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);
byte softApActive = 0; //is it in soft ap mode or not
byte prevApConns = 0;  //prv current number of clients connected to softap
byte nrConns = 0;

#include <Ticker.h>
Ticker ticker1Hz;
Ticker ticker10Hz;
Ticker ticker1kHz;

// vars used for internet radio (connection & string processing)
String strRequest ;    
String strAnswer ;
boolean endOfHeaders = false;
byte tmpChr;
unsigned int metaInt = 0;            // how many mp3 bytes are there between two metadata blocks (icy-metaint header = the number of bytes between 2 metadata chunks)
unsigned int metaLength = 0;         // length of metadata text
unsigned int i = 0;
String metaDataTxt_tmp ;
String metaDataTxt ="";
String metaStationName, metaBR;
int j = 0;
byte txtScroll = 0 ;        // text scroll counter
int  txtDir    = 1;         // scroll direction (-1 / +1)


#define DATA_BUFFER_SIZE      25000  // size of the circular buffer - used for storing mp3 data before sending to VC1053
byte byteBuffer[DATA_BUFFER_SIZE];
#define VS_BUFFER_SIZE        32     // number of bytes sent to VS1053 into one batch
byte vsBuffer[VS_BUFFER_SIZE];       // minibuffer for storing data sent to VS1053
unsigned int writePointer = 0;
unsigned int readPointer  = 0; 

const char* ntpServer = "pool.ntp.org";  // TO DO: store it into json and make it user selectable
const long  gmtOffset_sec = 7200;        // --- " --- " ---
const int   daylightOffset_sec = 3600;   
int  rtc_y = 0;
byte rtc_m = 0;
byte rtc_d = 0;
byte rtc_h = 0;
byte rtc_mn =0;
byte rtc_s = 0;
byte rtc_dw = 0;
struct tm rtc;  // rtc not to be confused with RTC
/*
struct tm // 0 = sunday
{
int    tm_sec;   //   Seconds [0,60]. 
int    tm_min;   //   Minutes [0,59]. 
int    tm_hour;  //   Hour [0,23]. 
int    tm_mday;  //   Day of month [1,31]. 
int    tm_mon;   //   Month of year [0,11]. 
int    tm_year;  //   Years since 1900. 
int    tm_wday;  //   Day of week [0,6] (Sunday =0)
int    tm_yday;  //   Day of year [0,365]. 
int    tm_isdst; //   Daylight Savings flag. 
}
*/  

unsigned long prvMillis = 0;
unsigned long likedSongMillis = 0;  // timer for debouncing pressed Like button


#ifdef __cplusplus      // necessary for reading temperature in esp32
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
float temp_celsius;

byte autoStartH ; // scheduler auto start & stop hours and minutes
byte autoStartM ;
byte autoStopH ;
byte autoStopM ;
byte autoDoW ;    // week days for scheduling; each bit is a weekday (bit 0 = day of week 0 .... bit 6 = day of week 6)

struct stations {
    String title, host, path;
    unsigned int port;
};
stations stationList[50] ;  // array of structs storing station list (after json parsing)
byte stationCnt = 0;
byte stationNow = 0 ;       // id-ul postului playat acum, din stationList
byte prevStationNow = 0;    // used to check if we've just changed the station; used in showStdStat(), in order to not clear screen if station is not changed
byte isClosing = 0;         // flag indicating that we're currently in process of changing stations and disconnecting, to prevent opening a connection while another one is closing





void setup() {
  pinMode(RELAYPIN, OUTPUT); digitalWrite(RELAYPIN, HIGH); // disconnect speakers, to prevent a "thumping" sound

  Serial.begin(115200); delay(100); Serial.println(F("\r\n\r\n------------------------- START -------------------------"));
  lcd.begin(); lcd.backlight(); lcd.clear(); lcd.setCursor(0,0); lcd.print("STARTING");
  
  jsonLoad(); 

  pinMode(BUTTON1, INPUT); pinMode(BUTTON2, INPUT); pinMode(BUTTON3, INPUT); pinMode(BUTTON4, INPUT);
  Serial.print(F("[setup] VS1053 GPIO test: ")); 
  pinMode(BREAKOUT_CS, OUTPUT); pinMode(BREAKOUT_DCS, OUTPUT); pinMode(DREQ, INPUT); pinMode(BREAKOUT_RESET, OUTPUT);
  if (!vsPlayer.begin()) { Serial.println(F("[ERROR] VS1053 not found. Program will continue ERRATICLY without sound.")); } else { Serial.println(F("[OK]"));  }  
  // TO DO: to set a flag for running without sound (to disable response check from VS)
   
  vsPlayer.softReset(); delay(150); 
  //sinetest is not really necessary
  //vsPlayer.sineTest(1000, 100); vsPlayer.sineTest(1100, 100); vsPlayer.sineTest(1200, 100); vsPlayer.sineTest(0, 100); 

  vsPlayer.sciWrite(VS1053_REG_VOLUME, VS_vol_l * 256 + VS_vol_r);  delay(10); // set vol & bass/treble adj with values read from json
  vsPlayer.sciWrite(VS1053_REG_BASS, SB_FREQLIMIT + SB_AMPLITUDE * 16 + ST_FREQLIMIT * 256 + ST_AMPLITUDE * 4096); delay(10);

  ticker1Hz.attach_ms (1000, func1Hz);  // very slow stuff like screen update
  ticker10Hz.attach_ms(200,  func10Hz); // keyboard processing, http server
  ticker1kHz.attach_ms(1,    func1kHz); // buffer feeding

  lcd.begin(); 
}





void loop() {    // slow stuff is done here

  wifiConn(); // checks wether it's connected to any wifi hotspot or not, tries to connect to 3 fav hotspots; if not succeed, then creates it's own soft AP
  
  if (fgApp == 0) { fgAppClock(); }
  if (fgApp == 1) { fgAppRadio(); }
  if (fgApp == 2) { fgAppList();  }
  if (fgApp == 3) { fgAppInfo(); }

}
