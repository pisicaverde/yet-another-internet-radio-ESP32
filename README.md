:baby: This is a work in progress. It may change or disappear anytime, or update irregularly. :baby:
# yet-another-internet-radio-ESP32
Streaming media player (internet radio) based on ESP32 and VS1053.
It plays anything that VS1053 may decode.

It's a follow-up of the previous [ESP8266 internet radio](https://github.com/pisicaverde/yet-another-internet-radio/), now defunct. 
This doc is rather a notice to myself, in case I'll try to debug it after a long time.

### Features ###
- tested against up to 192 kbps streams. It plays even MIDI files! (type 0 MIDI)
- controlled via LCD & 4 buttons and through a nice, html5, web interface
- time, stream title, current song, bitrate, wifi level, buffer level shown on LCD during playback
- detailed configuration is available using web interface
- :hearts: LIKE button, for saving "now playing" song title into flashmem. *Fave it now, google it later.*
- using MCU controlled relays for for delayed speaker coupling (to avoid the "thump" sound at power on)
- configuration file is stored as a json into SPIFFS
- at first run or corrupted spiffs, it will create a default json config
- in case of ESP32 crash & restart, it continues in the previous state
- scheduler for unattended use, with selectable running days and hours
- RTC using ESP32's internal clock, updated from NTP continuously
- bass, treble and separate left and right volume setting using VS1053 registers;
- wifiManager-like functionality - if none of saved APs is available, it creates its own, for accessing config page.

It's stable, all features are working, but with issues and lots of ToDo's.

### Component list: ###
- [ESP32 DevKitC VIB](https://www.tme.eu/en/details/esp32-devkitc-vib/development-tools-for-data-transmission/espressif/) 
- [VS1053 mp3 decoder](https://www.tme.eu/en/details/mikroe-946/add-on-boards/mikroelektronika/mp3-click/) (any direct 3.3V VS1053)
- I²C 20x4 chars LCD
- 4 buttons
- [2-relay module](https://www.banggood.com/2-Channel-Relay-Module-12V-with-Optical-Coupler-Protection-Relay-Extended-Board-For-Arduino-MCU-p-1399427.html)

### Configuration ###
All config is stored in the flash memory (/irconfig.json). At first run or if the file gets corrupted, it's overwriten with the default data. This allows you to connect to iRadio access point with 12345678 as password and make any changes:
```
{
  "user_agent": "iRadio",
  "ap_pass": "12345678",
  "wifi_channel": 10,
  "VS_vol_l": 50,
  "VS_vol_r": 70,
  "ST_AMPLITUDE": 15,
  "ST_FREQLIMIT": 8,
  "SB_AMPLITUDE": 15,
  "SB_FREQLIMIT": 15,
  "stationNow": 0,
  "fgApp": 0,
  "autoStartH": 8,
  "autoStartM": 0,
  "autoStopH": 18,
  "autoStopM": 0,
  "autoDoW": 127,
  "wifi": [
    {
      "ssid": "",
      "pass": ""
    },
    {
      "ssid": "",
      "pass": ""
    },
    {
      "ssid": "",
      "pass": ""
    }
  ],
  "station": [
    {
      "title": "Radio Shantz",
      "host": "82.78.220.54",
      "path": "/stream.mp3",
      "port": 8000
    }
  ]
}
```
### How it works ###
There can be created several "apps", but only one runs in foreground. Currently there are 4 apps:
1) clock (shown in stand by mode)
2) radio
3) station list
4) info (not really necessary, done just as a showcase)
Several can be added, for example a Bluetooth speaker app, MIDI player or a weatherundeground.com client.
You may switch between them from the web interface or by pushing `button4`.
