// foreground app 1 - clock


// TO DO: to add a clearscreen when changing stations


void fgAppRadio() {
      if (fgAppPrev != fgApp) { Serial.println(F("[fgAppRadio] Switching to Internet Radio")); fgAppPrev = fgApp; lcd.clear(); jsonSave(); 
      }

      if ( (WiFi.status() != WL_CONNECTED) ) { lcd.setCursor(0,1); lcd.print(F("NO WIFI FOR RADIO")); } // no wifi right now, but it should be handled soon in the main loop by wifiConn()

      else {
              if (millis() >= prvMillis + 1000) { radioUpdateScreen(); prvMillis = millis(); } // screen update

              if (!netClient.connected() && (isClosing != 1) ) {    // if not connected nor in process of disconnecting
                      char buf[50];
                      stationList[stationNow].host.toCharArray(buf, 50);  // wificlient.connect for esp32 accepts onlys char* as argument, so we have to convert it
                      
                      Serial.print(F("[fgAppRadio] CONNECTING TO (")); Serial.print(stationNow); Serial.print(") "); Serial.print(stationList[stationNow].title); 
                      Serial.print(F("\thttp://")); Serial.print(stationList[stationNow].host); Serial.print(":"); 
                      Serial.print(stationList[stationNow].port); Serial.println(stationList[stationNow].path);

                      // TO DO : to catch refused connections - currently crashes if ERR_CONNECTION_REFUSED
                      // TO DO : to catch timeouts and This site can’t be reached 198.105.223.94 took too long to respond.
                      
                      int connectResult = netClient.connect(buf, stationList[stationNow].port); // simply opening connection to remote host
                      Serial.print(F("[fgAppRadio] connectResult=")); Serial.print(connectResult); Serial.print(" ");

                      if (connectResult == 1)  { Serial.println(F(" [Connected]")) ; // if connected, composing the request string and sending it to remote host
                                                  strRequest = "GET " + stationList[stationNow].path + " HTTP/1.1\r\nUser-Agent: " + user_agent;
                                                  strRequest = strRequest + "\r\nHost: " + stationList[stationNow].host + "\r\nIcy-Metadata:1\r\nConnection: keep-alive\r\n\r\n";
                                                  Serial.println(F("[fgAppRadio] Sending request:")); Serial.println(strRequest);
                                                  netClient.print(strRequest);
                                                }
                                else {  // if it could not connect, trying with next station
                                        Serial.printf("[fgAppRadio] ERROR ClientConnect: %s , skipping to next.", connectResult); statNext(); return; }

                      // receiving something
                      Serial.println(F("[fgAppRadio] Answer:")); delay(1000);     // a short delay for receiving data
                      endOfHeaders = false; strAnswer = "";
                      while ( netClient.available() && (endOfHeaders == false) )  // reading until \r\n\r\n, then the stream follows
                            { tmpChr = netClient.read();
                              Serial.write(tmpChr);
                              strAnswer = strAnswer + char(tmpChr);
                              if (strAnswer.endsWith("\r\n\r\n")) { endOfHeaders = true; }
                            }

                      if (strAnswer.indexOf("ICY 401") != -1 ) { Serial.println(F("[fgAppRadio] 401 SERVICE UNAVAILABLE")); statNext(); return; }

                      // parsing some info from http headers
                      int paramStart, paramEnd;
                      paramStart = strAnswer.indexOf("icy-metaint:") + 12; paramEnd   = strAnswer.indexOf("\r\n", paramStart); metaInt = strAnswer.substring(paramStart, paramEnd).toInt();
                      paramStart = strAnswer.indexOf("icy-name:") + 9; paramEnd = strAnswer.indexOf("\r\n", paramStart); metaStationName = strAnswer.substring(paramStart, paramEnd);
                      paramStart = strAnswer.indexOf("icy-br:") + 7; paramEnd = strAnswer.indexOf("\r\n", paramStart); metaBR = strAnswer.substring(paramStart, paramEnd);
                      Serial.printf("\r\n[fgAppRadio] All headers received. MP3 data between metadata: metaInt = %u bytes\r\n", metaInt);

                      jsonSave();
                    }
                  // else it means we're connected, so we're reading until there is no data in the tcp buffer or no room left in the circular buffer   
                  while (netClient.available() && (  !((writePointer + 1) % DATA_BUFFER_SIZE == readPointer)    ) ) {read1byte(); }
              } 
      
    // radio specific keys
    if (keyLast == BUTTON1) { statPrev(); }
    if (keyLast == BUTTON3) { statNext(); }
    if (keyLast == BUTTON2) { likedSong(); } 
    keyReady = 1; // flag indicating we're ready for key processing
}




byte read1byte() {
// reading 1 byte and treating it according to its position in stream

    if (( i < metaInt ) && (  !((writePointer + 1) % DATA_BUFFER_SIZE == readPointer)    ) ) { // the mp3 data zone
             byteBuffer[writePointer] = netClient.read();
             //if (!(i%1024)) { Serial.print(F("#")); }           // print hash mark
             writePointer = (writePointer + 1) % DATA_BUFFER_SIZE;
             if (writePointer == readPointer) { readPointer = (readPointer + 1) % DATA_BUFFER_SIZE; } // if we're writing faster than reading, we should increment readPointer too
       } 
    
    if (i == metaInt) { metaLength = netClient.read() * 16;   // the byte indicating number of following metadata bytes
                        if (metaLength != 0) { Serial.printf("\r\n[read1byte] metadata: %u bytes ", metaLength); }}

    if ( (i > metaInt) && (i <= (metaInt + metaLength ) ) ) // meta data text (song title etc)
                     { char tmpChr = netClient.read() ; 
                       metaDataTxt_tmp += tmpChr; j++; 
                       if (j >= metaLength) {  // the metadata string is complete
                                               int metaDataStart = metaDataTxt_tmp.indexOf("='") + 2 ; 
                                               int metaDataEnd   = metaDataTxt_tmp.indexOf("';", metaDataStart);
                                               metaDataTxt = "      " + metaDataTxt_tmp.substring(metaDataStart, metaDataEnd) + "      ";// adding spaces just for nicer lcd scrolling
                                               j = 0 ; metaDataTxt_tmp = "";
                                               Serial.println(metaDataTxt);
                                               }
                     }
                     
    if ( i == (metaInt + metaLength +1) ) { i = 0; } // end of data frame?
                                     else   i++;
}





void stopDisconnect() {
  
    netClient.stop(); Serial.println("[stopDisconnect] closing NetClient"); delay(250);
    while(netClient.connected()) { if (netClient.available()) { netClient.read(); Serial.print("."); } } // the only way to completely empty the tcp buffer
    writePointer = 0;
    readPointer = 0;
    metaInt = 0;
    metaLength = 0;
    i = 0; 
    metaDataTxt = "";
    txtScroll = 0;
    txtDir    = 1; 
}




void statPrev() { // switching to previous station
  Serial.print(F("[statPrev] switching to prev - ")); 
  if (stationNow >= 1)    { Serial.println(F("[OK]")); stationNow-- ; stopDisconnect(); fgAppPrev = 100;} 
                     else { Serial.println(F("Already at first, switching to last.")); stationNow = stationCnt-1 ;}
  Serial.print(F("[statPrev] current station is: ")); Serial.println(stationNow);
  stopDisconnect(); 
}

void statNext() { // switching to next station
  Serial.print(F("[statNext] switching to next - ")); 
  if (stationNow < stationCnt-1) { Serial.println(F("[OK]")); stationNow++ ; stopDisconnect(); fgAppPrev = 100; } 
                            else { Serial.println(F("End of list, rewinding to first.")); stationNow = 0 ; }
  Serial.print(F("[statNext] current station is: ")); Serial.println(stationNow);
  stopDisconnect(); 
}


void likedSong() { // saves into spiffs the current playing title

  if (millis() < likedSongMillis + 2000) { return;}    // a long debounce (2 s)

  likedSongMillis = millis();
 
  Serial.println(F("[likedSong] Appending currently playing song to string."));
  if(!SPIFFS.begin(true)){ Serial.println(F("[JsonSetup] An Error has occurred while mounting SPIFFS, aborting.")); die(); }
  File file = SPIFFS.open("/songlist.txt", "a"); if(!file){ Serial.println(F("[jsonUpdate] Failed to open file for writing, aborting.")); die(); }
  String txt = "->" + metaDataTxt + "\r\n"; // TO DO: substring (3, n-3) ; TO DO: adding timestamps in front of song titles and station name
  file.print(txt);
  file.close();  
}


 
void radioUpdateScreen() {  // updating screen in internetradio mode

  // if we're during a likedSong debounce, we'll display only the confirmation message
  if (millis() < likedSongMillis + 2000) { lcd.clear(); lcd.setCursor(0,0); lcd.print("SAVED"); return;}   // TO DO: something nicer on display


  lcd.setCursor(0,0); lcd.printf("Station %u/%u", stationNow+1, stationCnt);
  lcd.setCursor(15,0); 
  
  lcd.print( ( rtc_h <= 9 ? "0" : "") +  String(rtc_h) + " " + ( rtc_mn <= 9 ? "0" : "") + String(rtc_mn) ); 
  if (rtc_s % 2) { lcd.setCursor(17,0); lcd.print(":"); } else { lcd.setCursor(17,0); lcd.print(" "); } 
  
  lcd.setCursor(0,1); lcd.print(metaStationName.substring(0,20));
  lcd.setCursor(0,2); lcd.print(metaDataTxt.substring(txtScroll, txtScroll+19));
  lcd.setCursor(0,3); lcd.print(metaBR); lcd.print("kbps  ");
  lcd.setCursor(8,3); lcd.print("B:"); lcd.print(map(usedBuffer(), 0, DATA_BUFFER_SIZE, 0, 99), DEC); lcd.print("% ");
  
  int db = WiFi.RSSI(); 
  int q;
  if (db >= -50) { q = 100; } else if (db <= -100) { q = 0 ;}
  else {q = 2 * ( WiFi.RSSI() + 100) ; } // q este RSSI in procente
  lcd.setCursor(14,3); lcd.printf("W:%u%%",q); 
  if (q < 100) lcd.print(" "); // to clear a residual % symbol

  if (txtScroll == 0) txtDir = 1;
  if (txtScroll == (metaDataTxt.length() - 19) ) txtDir = -1;
  txtScroll = txtScroll + txtDir;

}

