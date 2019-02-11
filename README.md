:baby: This is a work in progress. It may change or disappear anytime, or update irregularly. :baby:
# yet-another-internet-radio-ESP32
Streaming media player (internet radio) based on ESP32 and VS1053.
It plays anything that VS1053 may decode.

This is a follow-up of the previous [ESP8266 internet radio](https://github.com/pisicaverde/yet-another-internet-radio/), now defunct. 

### Component list: ###
- [ESP32 DevKitC VIB](https://www.tme.eu/en/details/esp32-devkitc-vib/development-tools-for-data-transmission/espressif/) 
- [VS1053 mp3 decoder](https://www.tme.eu/en/details/mikroe-946/add-on-boards/mikroelektronika/mp3-click/) (any direct 3.3V VS1053)
- I²C 20x4 chars LCD
- 4 buttons
- [2 relay module](https://www.banggood.com/2-Channel-Relay-Module-12V-with-Optical-Coupler-Protection-Relay-Extended-Board-For-Arduino-MCU-p-1399427.html)

### Features ###
- tested against up to 192 kbps streams. It plays midi files too! (type 0 midi)
- time, stream title, current song, bitrate, wifi level, buffer level shown on lcd during playback
- [detailed configuration is available using web interface](https://pisicaverde.github.io/yet-another-internet-radio-ESP32/html-test/index.html)
- :hearts: LIKE button, for saving "now playing" song title. Save it now, google it later.
- it may use relays for for delayed speaker coupling (to avoid the "thump" sound at power on)
- config json file is stored into SPIFFS
- in case of uninteded restart, it continues with the previous state
- scheduler for unattended use (selectable running days and hours)
- RTC using ESP32's internal clock
- access to bass/treble/volume setting using VS1053 registers
- wifiManager-like functionality - if none of saved APs is available, it creates its own, for accessing config page.
