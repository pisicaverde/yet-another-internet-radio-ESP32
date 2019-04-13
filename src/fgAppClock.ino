void fgAppClock() {
  // foreground app - clock  
  if (fgAppPrev != fgApp) { 
        // is this the first call ?
        Serial.println(F("[fgAppClock] Switching to clock")); 
        fgAppPrev = fgApp; 
        stopDisconnect(); 
        lcd.clear(); 
        jsonSave(); 
        }

  lcd.setCursor(0,0); lcd.print("0-clock: (stand by)"); delay(500);

  if (millis() >= prvMillis + 1000) { // since we can't put screen update into ticker funcs, we do it heren -- updating lcd each second

        lcd.setCursor(0,2); 
        switch(rtc_dw) {
          case 0: lcd.print(F("Sunday")); break;
          case 1: lcd.print(F("Monday")); break;
          case 2: lcd.print(F("Tuesday")); break;
          case 3: lcd.print(F("Wednesday")); break;
          case 4: lcd.print(F("Thursday")); break;
          case 5: lcd.print(F("Friday")); break;
          case 6: lcd.print(F("Saturday")); break;
          case 7: lcd.print(F("Sunday")); break;
          default: lcd.print(F("ERROR")); break;
        }
        lcd.print(" ");
        
        lcd.print(rtc_y); lcd.print("-"); 
        lcd.print((rtc_m <= 9) ? "0" : ""); lcd.print(rtc_m); lcd.print("-"); 
        lcd.print((rtc_d <= 9) ? "0" : ""); lcd.print(rtc_d);
        
        lcd.setCursor(0,3); 
        lcd.print((rtc_h <= 9)  ? "0" : ""); lcd.print(rtc_h); lcd.print(":"); 
        lcd.print((rtc_mn <= 9) ? "0" : ""); lcd.print(rtc_mn); lcd.print(":"); 
        lcd.print((rtc_s <= 9)  ? "0" : ""); lcd.print(rtc_s);                    
        lcd.print(F(" and ")); lcd.print(temp_celsius); lcd.print((char)223); lcd.print("C");

        prvMillis = millis();
      }
    
  keyReady = 1;
}


