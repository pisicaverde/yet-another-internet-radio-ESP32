
void func1kHz() { 
  
  if ( keyReady == 1 ) keyLast = keyRead(); 

  // feeding the VS buffer; if there is anything in the buffer, it will play it.
  if ((usedBuffer() >= VS_BUFFER_SIZE)) {       // if there is enough data to compose the VS_BUFFER,
    digitalWrite(RELAYPIN, LOW); // set relay
    while (!digitalRead(DREQ)) { delay(1); }     // if DREQ==low, there is something playing right now; waiting a few ms for vs internal buffer to get empty and be ready for data feed
    for (int j = 0; j < VS_BUFFER_SIZE ; j++) { // composing the data buffer that will be sent to VS
        vsBuffer[j] = byteBuffer[readPointer];
        readPointer = (readPointer + 1) % DATA_BUFFER_SIZE;
        }
    vsPlayer.playData(vsBuffer, VS_BUFFER_SIZE);    // sending vsbuffer to player
    }
    else digitalWrite(RELAYPIN, HIGH); // disconnect the speakers if there is nothing to be played
  
 }



void func10Hz() {
  server.handleClient();
 
  if (keyLast == BUTTON4) { fgAppPrev = fgApp; fgApp++; } 
  if (fgApp >= fgAppMax) { fgApp = 0; fgAppPrev = fgAppMax - 1 ; delay(100); }
  keyReady = 0; // will not procees further keystrokes until explicitely granted by fgApp

  // schedule ckeck
  if (WiFi.status() == WL_CONNECTED) updateRtc(); // update time only if there is wifi, otherwise it should not reflect actual time

}



void func1Hz() { 
  // can't refresh screen here --> kernel panic :(
  // very blocking - should remain here or commented out completely
   nrConns = WiFi.softAPgetStationNum();
   if (nrConns != prevApConns) { 
                 Serial.print("[LOOP] The number of clients has changed: "); Serial.println(nrConns); 
                 if ( (nrConns == 0) && (prevApConns == 1) ) { Serial.println("[LOOP] The last client quit. Switching to station mode."); softApActive = 0; }
                 prevApConns = nrConns; } 
 
  
  unsigned long timeNow   = rtc_h * 60 + rtc_mn;
  
  if (autoDoW != 0 )
                        {  // run schedule only if there is any day selected; no day selected = scheduling is disabled
                          if (bitRead(autoDoW, rtc_dw) && (timeNow >= (autoStartH * 60 + autoStartM)) && (timeNow < (autoStopH * 60 + autoStopM))) // are we during running hours?
                                         { 
                                          if ( fgApp != 1 )
                                                    {  // if it is NOT playing, and the SCHEDULER is enabled, then switch to internet radio
                                                       fgAppPrev = fgApp; fgApp = 1; 
                                                       Serial.printf("[func1Hz] It is %u:%u , should start\r\n", rtc_h, rtc_mn);
                                                    }
                                         } 
                                    else { if (fgApp == 1) { // if it's playing internet radio, always switch to pause
                                                            fgAppPrev = fgApp; fgApp = 0; 
                                                            Serial.printf("[func1Hz] It is %u:%u , should stop\r\n" , rtc_h, rtc_mn); 
                                                          }
                                         }
                          }
                     
}


