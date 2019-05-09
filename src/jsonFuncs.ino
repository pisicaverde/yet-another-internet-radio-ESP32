// TO DO: backup & restore for the json file



void jsonSave() {
  // serialising vars and saving json to spiffs
  // extremely slow and blocking ; it will interrupt playback for a moment; it can't be used in real time, simultaneously with ticker funcs

Serial.print(F("[jsonSave] Saving state & config updates to json..."));
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
{\"user_agent\":\"iRadio\",\"ap_pass\":\"12345678\",\"wifi_channel\":10,\"VS_vol_l\":50,\"VS_vol_r\":70,\"ST_AMPLITUDE\":15,\"ST_FREQLIMIT\":8,\"SB_AMPLITUDE\":15,\"SB_FREQLIMIT\":15,\"stationNow\":3,\"fgApp\":0,\"autoStartH\":8,\"autoStartM\":0,\"autoStopH\":18,\"autoStopM\":0,\"autoDoW\":127,\"wifi\":[{\"ssid\":\"wifi\",\"pass\":\"azero1234\"},{\"ssid\":\"\",\"pass\":\"\"},{\"ssid\":\"\",\"pass\":\"\"}],\"station\":[{\"title\":\"Radio Shantz\",\"host\":\"82.78.220.54\",\"path\":\"/stream.mp3\",\"port\":8000}]}\
"))   { Serial.println(F("[writeDefaultJson] File was written, you should reset now.")); }
 else { Serial.println(F("[writeDefaultJson] File write failed. Dunno what to do next.")); }

// TO DO: after a writeDefaultJson program should die with a message ; after reset, it will run normally with the new file

}

