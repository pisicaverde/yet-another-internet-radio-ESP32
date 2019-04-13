void fgAppList() {
  
  if (fgAppPrev != fgApp) { Serial.println("[fgAppList] Changing mode to LIST"); fgAppPrev = fgApp; stopDisconnect(); jsonSave(); scrUpdateList(); }
  keyReady = 1;

  if (stationNow != prevStationNow) {
           scrUpdateList();
           prevStationNow = stationNow;
  }

  if ((keyLast == BUTTON1) && (stationNow >= 1)) { Serial.println("[fgAppList] PREV button pressed"); prevStationNow = stationNow; stationNow-- ; delay(150); /* return; */ }
  if ((keyLast == BUTTON3) && (stationNow < stationCnt-1)) { Serial.println("[fgAppList] NEXT button pressed"); prevStationNow = stationNow; stationNow++ ; delay(150);  /* return; */ }

//  if (keyLast == BUTTON2)  { Serial.println("[fgAppList] PLAY"); fgApp = 1; fgAppPrev = 2; stopDisconnect(); delay(250); } 
}








void scrUpdateList() {

  lcd.clear();
  
  lcd.setCursor(0,0); lcd.print(F("1. Station list"));
  lcd.setCursor(0,1); lcd.print(" "); 
  if (stationNow > 0) { if (stationNow <= 9) lcd.print("0"); lcd.print(stationNow);  // leading zero
                        lcd.print(F("."));
                        lcd.setCursor(5,1); lcd.print(stationList[stationNow-1].title.substring(0,15)); 
                      } else { lcd.setCursor(0,1); lcd.print(F("BEGIN               ")); }
  
  lcd.setCursor(0,2); lcd.write(B01111110) ;            
  if (stationNow+1 <= 9) lcd.print("0"); lcd.print(stationNow+1); 
  lcd.print(F("."));
  lcd.setCursor(5,2); lcd.print(stationList[stationNow].title.substring(0,15));
  
  lcd.setCursor(0,3); lcd.print(" "); 
  if (stationNow < stationCnt-1 ) { if (stationNow+2 <= 9) lcd.print("0"); lcd.print(stationNow+2); 
                                    lcd.print(F("."));
                                    lcd.setCursor(5,3); lcd.print(stationList[stationNow+1].title.substring(0,15)); 
                                  } else { lcd.setCursor(0,3); lcd.print(F("END                 ")); }

  
}

