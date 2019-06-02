
void wifiConn(){
    // if not connected, nor in station mode, nor while connecting, will try 3 connections; if failed, it will set esp32 to softAp mode
    // calling this is is usually blocking (it may continue play until buffer gets empty

    if ( (WiFi.status() != WL_CONNECTED) && (softApActive == 0) )  // not connected to any AP nor AP MODE (usually after cold start or reset)
        { 
          WiFi.mode(WIFI_STA); // switch to station mode and tries connecting; TO DO: to add wifi connect timeout (sec) into json; TO DO: wifiMulti-like functionality
          if(!tryWifiConnect(wifi0_ssid, wifi0_pass, wifi_channel, 15) && 
             !tryWifiConnect(wifi1_ssid, wifi1_pass, wifi_channel, 15) && 
             !tryWifiConnect(wifi2_ssid, wifi2_pass, wifi_channel, 15) && 
             !tryWifiConnect("1", "azero1234", 10, 15)) 
                     { Serial.println("[wifiConn] All 3 APs returned error.");}
                
                else { //connected to one of the APs ; init and get the time
                        Serial.print(F("[wifiConn] Getting time from NTP... "));
                        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
                        updateRtc();                      
                        Serial.println(&rtc, "Success, it is %A, %d %B %Y %H:%M:%S");
                      }
          if (WiFi.status() == WL_DISCONNECTED )  { // if failed to connect to all 3 prev APs, create it's own AP ; 
            //TO DO: to display something on LCD when AP is created
            //TO DO: Crashes after creating AP or client connect
            //TO DO: problems in dhcp server, http server does not serve pages
               Serial.print(F("[wifiConn] CREATING AP "));
               WiFi.mode(WIFI_AP);
               Serial.print(WiFi.softAP(user_agent, ap_pass, wifi_channel, 0, 4) ? " Soft AP created, accepting connections." : " Error creating AP");
               Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? " Soft AP set." : " Soft AP set failed.");
               Serial.print(F("[wifiConn] Server IP = ")); Serial.println(WiFi.softAPIP()); 
               Serial.printf("[wifiConn] you may now connect to SSID %s with pass %s\r\n", user_agent, ap_pass);
               softApActive = 1;
        
          }
          server.on("/", handleRoot); server.begin(); Serial.println(F("[wifiConn] HTTP server started")); // registering http server. It has to be accessabile in both STA and softAP modes.
        }

}




boolean tryWifiConnect(char* ssid, char* pass, byte channel, byte sec) {  
  // this function is blocking !
  // TO DO: SKIP IF RSSI BELOW CERTAIN VAL ; check existence of specified APs before waiting 10s
  // TO DO: return if blank params are received (5 positions in json and only 2 used)
  // TO DO: add channel in json and html config for wifi connection
  // TO DO: to save the last ssid (it will connect faster next time)

if ( WiFi.status() == WL_CONNECTED) { Serial.println(F("[tryWifiConnect] already connected to an AP; skipping.")); return(1) ; } // if already connected

// Serial.println("HARD CONNECT, SKIPPING OTHERS"); WiFi.begin("ssid", "pass"); while (WiFi.status() != WL_CONNECTED) { Serial.print("X"); delay(500); } return(1);

  Serial.printf("[tryWifiConnect] Trying SSID:%s PASS:%s while %u s ", ssid, pass, sec);
  lcd.setCursor(0,3); lcd.print("Trying:             "); lcd.setCursor (7, 3); lcd.print(ssid); 

  int c = 0;
  unsigned long starttime = millis();

  WiFi.setHostname(user_agent); 
  WiFi.disconnect(); // just in case. It may avoid a known bug.
  WiFi.begin(ssid, pass);
  while ( (WiFi.status() != WL_CONNECTED) & (millis() < starttime + sec*1000) ) { // will exit this loop if connected or timeouted
    lcd.setCursor(19,3);
    switch(c) {
      case 0: lcd.print("-"); break;
      case 1: lcd.print((char)B01100000); break;
      case 2: lcd.print("|"); break;
      case 3: lcd.print("/"); break;
    };
    c++; if (c == 4) c = 0;
    delay(250);
    Serial.print(".");
  }
  delay(100);
  if (WiFi.status() == WL_CONNECTED) { IPAddress ip = WiFi.localIP(); Serial.print(F("\r\n[tryWifiConnect] Success; my IP is ")); Serial.println(ip) ; return(1); }
                                else { Serial.println(F("\r\n[tryWifiConnect] Error connecting to this AP (timeout, incorrect password or unknown encription)")); return(0); }
}


