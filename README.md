:baby: This is a work in progress. It may change or disappear anytime, or update irregularly. :baby:
# yet-another-internet-radio-ESP32
Streaming media player (internet radio) based on ESP32 and VS1053

This is a follow-up of the ESP8266 internet radio (https://github.com/pisicaverde/yet-another-internet-radio/)

### Component list: ###
- [ESP32 DevKitC VIB](https://www.tme.eu/en/details/esp32-devkitc-vib/development-tools-for-data-transmission/espressif/) with 8MB flash, 8 MB RAM from which 4 MB addressable - to avoid the need for external RAM buffer
- [VS1053 mp3 decoder](https://www.tme.eu/en/details/mikroe-946/add-on-boards/mikroelektronika/mp3-click/) (any direct 3.3V VS1053)
- I²C RTC with DS3234
- I²C 20x4 chars LCD
- 4 buttons
- 2 relays for delayed speaker coupling (to avoid the "thump" sound when powerin on)

### Schematics: ###
Please check [the schematics](/yet-another-internet-radio-ESP32/Schematic_yet-another-internet-radio-ESP32_20181230134413-1.png).
