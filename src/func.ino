
void jsonSave() {
  // serialising vars and saving json to spiffs
  // extremely slow and blocking ; it will interrupt playback for a moment; it can't be used in real time, simultaneously with ticker funcs

Serial.print(F("[jsonSave] saving data to json..."));
  DynamicJsonBuffer jsonBufferX(10000);
  JsonObject& root = jsonBufferX.createObject();

  root["user_agent"]   = user_agent;
  root["ap_pass"]      = ap_pass;
  root["wifi_channel"] = wifi_channel ;
  root["VS_vol_l"]     = VS_vol_l;
  root["VS_vol_r"]     = VS_vol_r;
  root["ST_AMPLITUDE"] = ST_AMPLITUDE;
  root["ST_FREQLIMIT"] = ST_FREQLIMIT;
  root["SB_AMPLITUDE"] = SB_AMPLITUDE;
  root["SB_FREQLIMIT"] = SB_FREQLIMIT;
  root["stationNow"]   = stationNow;

  root["fgApp"]      = fgApp;
  root["autoStartH"] = autoStartH;
  root["autoStartM"] = autoStartM;
  root["autoStopH"]  = autoStopH;
  root["autoStopM"]  = autoStopM;
  root["autoDoW"]    = autoDoW;

  JsonArray& wifi = root.createNestedArray("wifi");
  JsonObject& wifi_0 = wifi.createNestedObject(); wifi_0["ssid"] = wifi0_ssid; wifi_0["pass"] = wifi0_pass;
  JsonObject& wifi_1 = wifi.createNestedObject(); wifi_1["ssid"] = wifi1_ssid; wifi_1["pass"] = wifi1_pass;
  JsonObject& wifi_2 = wifi.createNestedObject(); wifi_2["ssid"] = wifi2_ssid; wifi_2["pass"] = wifi2_pass;

  JsonArray& station = root.createNestedArray("station");
  for (byte tmp = 0; tmp < stationCnt ; tmp++) {
    JsonObject& station_n = station.createNestedObject();
    station_n["title"] = stationList[tmp].title;
    station_n["host"]  = stationList[tmp].host;
    station_n["path"]  = stationList[tmp].path;
    station_n["port"]  = stationList[tmp].port;
  }
  
  // writing to spiffs
  if(!SPIFFS.begin(true)){ Serial.println(F("[jsonSave] An Error has occurred while mounting SPIFFS")); die(); }
  File file = SPIFFS.open("/irconfig.json", "w"); if(!file) { Serial.println(F("[jsonSave] Failed to open file for writing.")); die(); }
  root.printTo(file); 
  file.close();  
  
  //root.printTo(Serial);
  Serial.println(" [FINISHED]");

}





boolean jsonLoad() {
  
  DynamicJsonBuffer jsonBuffer(10000);
  char json[5000];
  int i = 0;
  
  if(!SPIFFS.begin(true)){ Serial.println(F("[jsonLoad] An Error has occurred while mounting SPIFFS")); die(); }
  
  File file = SPIFFS.open("/irconfig.json", "r"); if(!file){ Serial.println("[JsonSetup] Failed to open file for reading"); die(); }
  
  while(file.available()) { json[i++] = file.read();
                            Serial.print(json[i]);
                          }
  file.close();

  JsonObject& root = jsonBuffer.parseObject(json); 
  if (!root.success()) { Serial.println(F("[JsonSetup] parseObject() failed")); writeDefaultJson(); die(); } 
                  else { Serial.println(F("[JsonSetup] Json successfully parsed."));
                         //root.printTo(Serial);
                       }

  wifi_channel = root["wifi_channel"];
  VS_vol_l = root["VS_vol_l"];
  VS_vol_r = root["VS_vol_r"];
  ST_AMPLITUDE = root["ST_AMPLITUDE"];
  ST_FREQLIMIT = root["ST_FREQLIMIT"];
  SB_AMPLITUDE = root["SB_AMPLITUDE"];
  SB_FREQLIMIT = root["SB_FREQLIMIT"];
  stationNow   = root["stationNow"];
  fgApp        = root["fgApp"];
  autoStartH   = root["autoStartH"];
  autoStartM   = root["autoStartM"];
  autoStopH    = root["autoStopH"];
  autoStopM    = root["autoStopM"];
  autoDoW      = root["autoDoW"];

  strcpy(user_agent, root["user_agent"]);
  strcpy(ap_pass,    root["ap_pass"]);
  
  JsonArray& wifi = root["wifi"];
  strcpy(wifi0_ssid, wifi[0]["ssid"]); strcpy(wifi0_pass, wifi[0]["pass"]);   
  strcpy(wifi1_ssid, wifi[1]["ssid"]); strcpy(wifi1_pass, wifi[1]["pass"]);   
  strcpy(wifi2_ssid, wifi[2]["ssid"]); strcpy(wifi2_pass, wifi[2]["pass"]);   

  JsonArray& stations = root["station"];
  stationCnt = stations.size(); 
  Serial.printf("[jsonSetup] found %u stations in JSON.\r\n", stationCnt);
  
  for (byte tmp = 0; tmp < stationCnt ; tmp++) {
    JsonObject& tmpStation = stations[tmp];
    char tmpStationTitle[50]; strcpy(tmpStationTitle, tmpStation["title"]);
    char tmpStationHost[50];  strcpy(tmpStationHost,  tmpStation["host"]);
    char tmpStationPath[50];  strcpy(tmpStationPath,  tmpStation["path"]);
    char tmpStationPort[50];  strcpy(tmpStationPort,  tmpStation["port"]);

    stationList[tmp].title = tmpStationTitle;
    stationList[tmp].host = tmpStationHost;
    stationList[tmp].path = tmpStationPath;
    stationList[tmp].port = String(tmpStationPort).toInt();

    // Serial.print("\t"); Serial.print(tmp); Serial.print(" "); Serial.print(stationList[tmp].title); Serial.print(" ");   Serial.print(stationList[tmp].host); Serial.print(":");  
    // Serial.print(stationList[tmp].port) ; Serial.println(stationList[tmp].path); 
  }

}








void writeDefaultJson() {
  if(!SPIFFS.begin(true)){ Serial.println(F("[writeDefaultJson] An Error has occurred while mounting SPIFFS")); die(); }
  File file = SPIFFS.open("/irconfig.json", "w"); if(!file){ Serial.println(F("[writeDefaultJson] Failed to open file for writing")); die(); }

  if(file.print("\
{\"user_agent\":\"iRadio\",\"ap_pass\":\"12345678\",\"wifi_channel\":10,\"VS_vol_l\":50,\"VS_vol_r\":70,\"ST_AMPLITUDE\":15,\"ST_FREQLIMIT\":8,\"SB_AMPLITUDE\":15,\"SB_FREQLIMIT\":15,\"stationNow\":0,\"fgApp\":0,\"autoStartH\":8,\"autoStartM\":0,\"autoStopH\":18,\"autoStopM\":0,\"autoDoW\":127,\"wifi\":[{\"ssid\":\"YOURWIFI\",\"pass\":\"YOURPASS\"},{\"ssid\":\"\",\"pass\":\"\"},{\"ssid\":\"\",\"pass\":\"\"}],\"station\":[{\"title\":\"Radio Shantz\",\"host\":\"82.78.220.54\",\"path\":\"/stream.mp3\",\"port\":8000}]}\
"))   { Serial.println(F("[writeDefaultJson] File was written, you should reset now.")); }
 else { Serial.println(F("[writeDefaultJson] File write failed. Dunno what to do next.")); }

}







void wifiConn(){
    // if not connected, nor in station mode, will try 3 connections; if failed, it will set esp32 to softAp mode
    
    if ( (WiFi.status() != WL_CONNECTED) && (softApActive == 0) )  // not connected to any AP nor AP MODE (usually after cold start or reset)
        { 
          WiFi.mode(WIFI_STA); // switch to station mode and tries connecting           
          if(!tryWifiConnect(wifi0_ssid, wifi0_pass, wifi_channel) && 
             !tryWifiConnect(wifi1_ssid, wifi1_pass, wifi_channel) && 
             !tryWifiConnect(wifi2_ssid, wifi2_pass, wifi_channel)) { Serial.println("[wifiConn] All 3 APs returned error.");}
              else { //init and get the time
                      Serial.print(F("[wifiConn] Getting time from NTP... "));
                      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
                      updateRtc();                      
                      Serial.println(&rtc, "[OK] it is %A, %d %B %Y %H:%M:%S");
                    }
          if (WiFi.status() == WL_DISCONNECTED )  { // if failed to connect to all 3 prev APs, create it's own AP
               Serial.print("CREATING AP ");
               WiFi.mode(WIFI_AP);
               Serial.print(WiFi.softAP(user_agent, ap_pass, wifi_channel, 0, 4) ? " Soft AP created, accepting connections." : " Error creating AP");
               Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? " Soft AP set." : " Soft AP set failed.");
               Serial.print(F("[LOOP] Server IP = ")); Serial.println(WiFi.softAPIP()); 
               Serial.printf("[LOOP] connect to SSID %s with pass %s\r\n", user_agent, ap_pass);
               softApActive = 1;
        
          }
               server.on("/", handleRoot); server.begin(); Serial.println(F("[wifiConn] HTTP server started")); // registering http server
        }

}




boolean tryWifiConnect(char* ssid, char* pass, byte sec) {  
  // TO DO: SKIP IF RSSI BELOW CERTAIN VAL ; check existence of specified APs before waiting 10s
  // TO DO: return if blank params are received (5 positions in json and only 2 used)
  // TO DO: add channel in json and html config for wifi connection

  Serial.printf("[tryWifiConnect] Trying SSID:%s PASS:%s while %u s ", ssid, pass, sec);
  if ( WiFi.status() == WL_CONNECTED) { Serial.println("[ALREADY+SKIP]"); return(0) ; } // if already connected

  lcd.setCursor(0,3); lcd.print("Trying:             "); lcd.setCursor (7, 3); lcd.print(ssid); 

  int c = 0;
  unsigned long starttime = millis();

  WiFi.setHostname(user_agent);
  WiFi.begin(ssid, pass);

  while ( (WiFi.status() != WL_CONNECTED) & (millis() < starttime + sec*1000) ) {
    lcd.setCursor(19,3);
    switch(c) {
      case 0: lcd.print("-"); break;
      case 1: lcd.print((char)B01100000); break;
      case 2: lcd.print("|"); break;
      case 3: lcd.print("/"); break;
    };
    c = c+1; if (c == 4) c = 0;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {   IPAddress ip = WiFi.localIP(); Serial.print(F(" [CONNECTED] my IP is ")); Serial.println(ip) ; return(1); }
    else { Serial.println(F(" [ERROR]")); return(0); }
}





void die() { 
  // TO DO: this should be replaced with a more elegant solution
  Serial.println(F("[die] DEAD BY AN ERROR")); while (1) {yield();} 
  } 




int usedBuffer() {
  // the function returns the number of bytes in use in circular buffer
  return(( DATA_BUFFER_SIZE + writePointer - readPointer ) % DATA_BUFFER_SIZE); 
  }




byte keyRead(){ 
 if (keyReady == 1) {// if the previus keypress was treated
                     if (digitalRead(BUTTON1)) return(BUTTON1);
                     if (digitalRead(BUTTON2)) return(BUTTON2);
                     if (digitalRead(BUTTON3)) return(BUTTON3);
                     if (digitalRead(BUTTON4)) return(BUTTON4);
                    }
 return(0) ; 
}



void updateRtc() {
  if(!getLocalTime(&rtc)){ Serial.println("[updateRtc] Failed to obtain time"); return; }
  rtc_y  = rtc.tm_year + 1900;
  rtc_m  = rtc.tm_mon + 1;
  rtc_d  = rtc.tm_mday;
  rtc_h  = rtc.tm_hour;
  rtc_mn = rtc.tm_min;
  rtc_s  = rtc.tm_sec;
  rtc_dw = rtc.tm_wday + 0;
  temp_celsius = ( temprature_sens_read() - 32 ) / 1.8;
}



