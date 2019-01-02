/*
      YET-INTERNET-RADIO-WITH-ESP32
      *** POWER ON SELF TEST ***

      Schematics: https://github.com/pisicaverde/yet-another-internet-radio-ESP32/blob/master/Schematic_yet-another-internet-radio-ESP32_20181230134413-1.png

      It performs a quick check whether all hardware is properly installed and runs ok.
      - ESP32 capabilities (RAM & flash size, wifi connection)
      - VS1053 connection
      - RTC clock
      - LCD display
      - relay command
      - keyboard (4 buttons)
      - wifi connection

*/



#include <Arduino.h>
#include <WiFi.h>

#include <Adafruit_VS1053.h>    // don't forget to connect miso/mosi/sck
#define BREAKOUT_CS     5       // VS1053 chip select pin (output) = Control Chip Select Pin (for accessing SPI Control/Status registers) --> XCS
#define BREAKOUT_DCS    33      // VS1053 Data/chip/command select pin (output) --> XDSV sau XDCS
#define DREQ            35      // VS1053 Data request, ideally an Interrupt pin: Player asks for more  data, --> DREQ (esp: GPIO10 = SD3
#define BREAKOUT_RESET  -1      // VS1053 reset pin (output) --> XRET
#define CARDCS          -1      // SD Card command select pin (output)
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

#include <Wire.h>

#include <LiquidCrystal_I2C.h>  // lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);

#include "uRTCLib.h"
uRTCLib rtc;

#define BUTTON1 32
#define BUTTON2 27
#define BUTTON3 14
#define BUTTON4 12
#define RELAYPIN 15  // pull up!



void setup() {

  Serial.begin(115200);
  Serial.println(F("\r\n---------------\r\nHardware check:\r\n---------------\r\n"));



  // ESP32 capabilities:
  if (psramFound()) {
    Serial.println("PSRAM was found and loaded");
  } else {
    Serial.println("NO EXTRA PSRAM");
  }
  Serial.printf("Free Heap: %u Bytes\r\n", ESP.getFreeHeap() );
  Serial.printf("CPU speed: %u MHz\r\n", ESP.getCpuFreqMHz() );
  Serial.printf("SDK vers.: %s\r\n", ESP.getSdkVersion() );



  // wifi scan & show rssi level
  Serial.print(F("\r\n--> Wifi scan start... "));
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  Serial.print(F("[DONE] "));
  if (n == 0) {
    Serial.println(F("no networks found"));
  } else {
    Serial.print(n);
    Serial.println(F(" networks found"));
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm)");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " OPEN" : "*");
      delay(10);
    }
  }
  Serial.println(F("\r\nNotes:\r\n-30 dBm: AMAZING! max achivable\r\n-67 dBm: Very good, suitable for streaming\r\n-70 dBm: Okay, minimum signal strenth for reliable delivery\r\n-80, -90 dBm: not good / unusable"));



  // VS1053 test
  Serial.print(F("\r\n--> VS1053 GPIO test: "));
  if (!musicPlayer.begin()) {
    Serial.println(F("[ERROR] VS1053 not found."));
  } else {
    Serial.println(F("[OK] you should hear a beep"));
  }
  musicPlayer.softReset(); delay(150);
  musicPlayer.setVolume(0, 0);     // 0 = max vol
  musicPlayer.sineTest(1000, 100); musicPlayer.sineTest(1100, 100); musicPlayer.sineTest(1200, 100); musicPlayer.sineTest(0, 100);// frequency in Hz, duration in milliseconds



  // Buttons setup; since there are plenty of GPIOs, we may use one pin per button
  pinMode(BUTTON1, INPUT); 
  pinMode(BUTTON2, INPUT); 
  pinMode(BUTTON3, INPUT); 
  pinMode(BUTTON4, INPUT); 



  // LCD test
  lcd.begin(); lcd.backlight(); lcd.clear();
  lcd.setCursor(0, 0); lcd.print(F(" Internet Radio 2.0"));
  lcd.setCursor(0, 1); lcd.print(F(__DATE__)); lcd.print(" "); lcd.print(F(__TIME__));



  // relay command pin & testing
  pinMode(RELAYPIN, OUTPUT); 
  digitalWrite(RELAYPIN, HIGH); 
  delay(500); 
  digitalWrite(RELAYPIN, LOW); 



  //set RTC if needed
  //second, minute, hour, dayOfWeek (1=Sunday, 7=Saturday), dayOfMonth (1 to 31), month, year (0 to 99)
  //rtc.set(0, 51, 19, 1, 1, 1, 19);



  //connecting to wifi
  Serial.print(F("\r\n--> Connecting to WiFi "));
  WiFi.begin("#############", "#############");    // <---- ssid / password here
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F(" [Connected]\r\n"));

}




void loop() {

  // button check
  Serial.print(BUTTON1); Serial.print("\t");
  Serial.print(BUTTON1); Serial.print("\t");
  Serial.print(BUTTON1); Serial.print("\t");
  Serial.print(BUTTON1);

  rtc.refresh();
  Serial.print(F("\tRTC: 20")); Serial.print(rtc.year());  Serial.print("-");  Serial.print(rtc.month());  Serial.print("-");  Serial.print(rtc.day());
  Serial.print("\t");           Serial.print(rtc.hour());  Serial.print(':');  Serial.print(rtc.minute());  Serial.print(':');  Serial.print(rtc.second());
  Serial.print(F(" DOW="));     Serial.print(rtc.dayOfWeek());
  Serial.print(F(" Temp="));    Serial.println(rtc.temp());

  delay(1000);
}
