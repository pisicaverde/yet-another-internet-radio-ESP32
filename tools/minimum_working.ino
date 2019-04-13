/*
      YET-INTERNET-RADIO-WITH-ESP32
      *** MINIMUM WORKING EXAMPLE ***
      It just connects to a station and plays something.
      There is NO BUFFERING in this example, it relays only on network speed.
      
*/


#include <Arduino.h>

#include <Adafruit_VS1053.h>    // don't forget to connect miso/mosi/sck
#define BREAKOUT_CS     5       // VS1053 chip select pin (output) = Control Chip Select Pin (for accessing SPI Control/Status registers) --> XCS
#define BREAKOUT_DCS    33      // VS1053 Data/chip/command select pin (output) --> XDSV sau XDCS
#define DREQ            35      // VS1053 Data request, ideally an Interrupt pin: Player asks for more  data, --> DREQ
#define BREAKOUT_RESET  -1      // VS1053 reset pin (output) --> XRET ; not necessary with esp32, wired to EN pin on ESP32
#define CARDCS          -1      // SD Card command select pin (output); not used
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

#include <WiFi.h>
WiFiClient       mp3client ;

#define DATA_BUFFER_SIZE      33     // size of the circular buffer used for storing mp3 data before sending to VC1053
byte byteBuffer[DATA_BUFFER_SIZE];
unsigned int writePointer = 0;
unsigned int readPointer  = 0;     

#define VS_BUFFER_SIZE        32     // number of bytes sent to VS1053 into one batch
byte vsBuffer[VS_BUFFER_SIZE];       // minibuffer for storing data sent to VS1053

boolean endOfHeaders = false;
byte tmpChr;
unsigned int metaInt = 0;            // how many mp3 bytes are there between two metadata blocks (icy-metaint header = the number of bytes between 2 metadata chunks)
unsigned int metaLength = 0;         // length of metadata text
unsigned int i = 0;
String strRequest ;
String strAnswer ;
#define HASHSIZE   1024              // bytes/hash mark printed in Serial monitor

const char * playServer     = "89.238.227.6";
const char * playPath       = "/" ;
const unsigned int playPort = 8346;

String metaDataTxt_tmp ;
String metaDataTxt ;
int j = 0;

// equaliser settings
byte ST_AMPLITUDE = 15; // Treble Control in 1.5 dB steps (-8...7, 0 = disabled)
byte ST_FREQLIMIT = 8;  // Lower limit frequency in 1000 Hz steps (1...15); the enhancement begins from this value and above (10 for 1 kHz etc), up to 15 kHz
byte SB_AMPLITUDE = 15; // Bass Enhancement in 1 dB steps (0..15, 0 = disabled)
byte SB_FREQLIMIT = 15; // Lower limit frequency in 10 Hz steps (2...15) ; set to roughly 1.5 times the lowest frequency the user’s audio system can reproduce (7 for 70 Hz etc), up to 150 Hz
byte VOL_L = 50 ; // 0 = max, 255 = analog shutdown, 254 = total silence; 20 is set to avoid clipping
byte VOL_R = 70 ; 



int usedBuffer() { return(( DATA_BUFFER_SIZE + writePointer - readPointer ) % DATA_BUFFER_SIZE); }




void setup() {

  Serial.begin(115200);
  Serial.println(F("\r\nInternet Radio Minimum Working Example:\r\n"));

  // VS1053 test
  Serial.print(F("--> VS1053 GPIO test: "));
  pinMode(BREAKOUT_CS, OUTPUT); pinMode(BREAKOUT_DCS, OUTPUT); pinMode(DREQ, INPUT); pinMode(BREAKOUT_RESET, OUTPUT);
  if (!musicPlayer.begin()) { Serial.println(F("[ERROR] VS1053 not found.")); while (1) { yield(); } } else { Serial.println(F("[OK]"));  }
  
  musicPlayer.softReset(); delay(150);
  musicPlayer.sineTest(1000, 100); musicPlayer.sineTest(1100, 100); musicPlayer.sineTest(1200, 100); musicPlayer.sineTest(0, 100); 
  //sineTest (frequency [Hz], duration [ms]) ; sineTest should be run before any volume or bass setting, since it resets VS1053 and all previous settings are lost

  //setting volume and bass/treble; converting bytes to uint16_t (volume and bass registers are 16 bit)
  noInterrupts();
  musicPlayer.sciWrite(VS1053_REG_VOLUME, VOL_L*256 + VOL_R);  delay(10); 
  musicPlayer.sciWrite(VS1053_REG_BASS, SB_FREQLIMIT + SB_AMPLITUDE * 16 + ST_FREQLIMIT * 256 + ST_AMPLITUDE * 4096); delay(10);
  interrupts();
  Serial.print(F("Volume=0x")); Serial.println(musicPlayer.sciRead(VS1053_REG_VOLUME), HEX);
  Serial.print(F("  Tone=0x")); Serial.println(musicPlayer.sciRead(VS1053_REG_BASS), HEX);
  
  //connecting to wifi
  Serial.print(F("--> Connecting to WiFi "));
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin("##########", "##########");  // PUT YOURS HERE
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print(F(" Connected with RSSI "));  Serial.print(WiFi.RSSI());  Serial.println(F(" dBm (should be: -30 ... -70 dBm)"));

  //connecting to stream
  Serial.print(F("--> Connecting to stream at ")) ; Serial.print(playServer) ; Serial.print(":") ; Serial.print(playPort) ; Serial.print(playPath) ;
  int connectResult = mp3client.connect(playServer, playPort);
  if (connectResult)  { Serial.println(F(" [Connected]")) ;
                        strRequest = "GET " + String(playPath) + " HTTP/1.1\r\n" + "User-Agent: ESP32_webradio\r\n" + "Host: " + playServer + "\r\n" + "Icy-Metadata:1\r\n" + "Connection: keep-alive\r\n\r\n";
                        Serial.println(F("--> Request is: ")); Serial.println(strRequest);
                        mp3client.print(strRequest);
                        Serial.println(F("--> Headers sent"));
                      }
                 else { Serial.print(F("--> ERROR ClientConnect: "));  Serial.print(connectResult); Serial.println(". Program stopped."); mp3client.stop();  while (1) { yield(); } }
  
  Serial.println(F("--> Receiving answer headers")); delay(2000); // a short delay for receiving data
  endOfHeaders = false; strAnswer = "";
  while ( mp3client.available() && (endOfHeaders == false) )      // reading until \r\n\r\n, then the stream follows
        { tmpChr = mp3client.read();
          Serial.write(tmpChr);
          strAnswer = strAnswer + char(tmpChr);
          if (strAnswer.endsWith("\r\n\r\n")) { endOfHeaders = true; }
        }
  Serial.println(F("\r\n--> All headers received."));

  int metaIntStart = strAnswer.indexOf("icy-metaint:") + 12;
  int metaIntEnd   = strAnswer.indexOf("\r\n", metaIntStart);
  metaInt = strAnswer.substring(metaIntStart, metaIntEnd).toInt();

  Serial.print(F("--> MP3 data bytes between metadata blocks: metaInt = ")); Serial.println(metaInt);
  Serial.print(F("--> Printing # for each ")); Serial.print(HASHSIZE); Serial.println(F(" downloaded bytes."));

}






/* 
 *    ICECAST STREAM structure:
 *    ... | <----- mp3 data, length: IcyMetaInt bytes--------> | [1 byte length specifier] | <--- text metadata---> | ...
 * 
 */

void loop() {

  if (mp3client.available()) { // if there is any available byte in the stream
    
        // if it's in the mp3 data zone and buffer is not full
        if (( i < metaInt ) && (  !((writePointer + 1) % DATA_BUFFER_SIZE == readPointer)    ) ) { 
           byteBuffer[writePointer] = mp3client.read();           // reading 1 byte from tcp stream
           if (!(i%HASHSIZE)) { Serial.print(F("#")); }           // print hash mark
           writePointer = (writePointer + 1) % DATA_BUFFER_SIZE;  // increment writePointer or set to 0 if it's on the last position
           if (writePointer == readPointer) { readPointer = (readPointer + 1) % DATA_BUFFER_SIZE; } // if we're writing faster than reading, we should increment readPointer too
        } 
        
        // checking whether we're on the length specifier char
        if (i == metaInt) { metaLength = mp3client.read() * 16;   // multiplied by 16 = how many metainfo bytes will follow 
                            if (metaLength != 0) {Serial.print(F("\r\n--> ")); Serial.print(metaLength); Serial.print(F(" Bytes of metadata: ")); } }

        // we're into the meta zone ; reading text medatadata, no audio data here
        if ( (i > metaInt) && (i <= (metaInt + metaLength ) ) ) 
                         { char tmpChr = mp3client.read() ; 
                           metaDataTxt_tmp += tmpChr; j++; 
                           if (j >= metaLength) {  // the metadata string is complete
                                                   int metaDataStart = metaDataTxt_tmp.indexOf("='") + 2 ; int metaDataEnd = metaDataTxt_tmp.indexOf("';", metaDataStart);
                                                   metaDataTxt = metaDataTxt_tmp.substring(metaDataStart, metaDataEnd);
                                                   j = 0 ; metaDataTxt_tmp = "";
                                                   Serial.println(metaDataTxt);
                                                   }
                         }

        // checking whether we have finished reading a cycle of mp3 data, control char, meta text
        if ( i == (metaInt + metaLength +1) ) { i = 0; Serial.println(); } 
                                         else   i++;
            
        } else { yield(); } ; // what to do if no data is received from tcp buffer (connection problems etc)


  if ((usedBuffer() >= VS_BUFFER_SIZE)) {         // if there is enough data to compose the VS_BUFFER,

      while (!digitalRead(DREQ)) { yield(); }     // if DREQ==low, there is something playing right now; waiting a few ms for vs internal buffer to get empty and be ready for data feed

      for (int j = 0; j < VS_BUFFER_SIZE ; j++) { // composing the data buffer that will be sent to VS
          vsBuffer[j] = byteBuffer[readPointer];
          readPointer = (readPointer + 1) % DATA_BUFFER_SIZE;
          }
      musicPlayer.playData(vsBuffer, VS_BUFFER_SIZE);    // sending vsbuffer to player
      } 

     // if the stream is down, we stop everything
     if (!mp3client.connected()) { Serial.println(F("\r\n\r\n--> DISCONNECTED from stream. Program stopped.")); mp3client.stop(); while(1){yield();} } 

}
