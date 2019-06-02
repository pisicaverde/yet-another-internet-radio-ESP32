// foreground app 0 - clock

void fgAppClock() {
  if (fgAppPrev != fgApp) { 
        // is this the first call ?
        Serial.println(F("[fgAppClock] Switching to clock")); 
        fgAppPrev = fgApp; 
        stopDisconnect(); 
        lcd.clear(); 
        jsonSave(); 
        delay(100);
        }

  if (digitalRead(BUTTON4)) fgAppSwitch();

  if (millis() < prvMillis + 1000) return;

  // since we can't put screen update into ticker funcs, we do it here -- updating lcd each second
  lcd.setCursor(0,0);
  lcd.print(rtc_y); lcd.print("-"); 
  lcd.print((rtc_m <= 9) ? "0" : ""); lcd.print(rtc_m); lcd.print("-"); 
  lcd.print((rtc_d <= 9) ? "0" : ""); lcd.print(rtc_d);
  lcd.setCursor(0,1); 
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
  
  String s = ((rtc_h <= 9)  ? "0" : "") + String(rtc_h) + ":" + ((rtc_mn <= 9) ? "0" : "") + String(rtc_mn) + ":" + ((rtc_s <= 9)  ? "0" : "") + rtc_s;
  lcdBigPrintAt(12,0,s);

   if (softApActive == 1) { lcd.setCursor(0,2); lcd.print("AP ("); lcd.print(nrConns); lcd.print(") "); lcd.print(user_agent); lcd.print("\"/"); lcd.print(ap_pass);
                            lcd.setCursor(0,3); lcd.print("IP:"); lcd.print(WiFi.softAPIP()); }
      
              else {  lcd.setCursor(0,2);
                      lcd.print("ST:"); lcd.print(WiFi.SSID()); 
                      lcd.setCursor(0,3); lcd.print("IP:"); IPAddress ip = WiFi.localIP();  lcd.print(ip); 
                   }

  lcd.setCursor(16,2); lcd.print((int)temp_celsius); lcd.print((char)223); lcd.print("C");
  prvMillis = millis();
   
}



void lcdBigPrintAt(byte x, byte y, String s) {  
  byte ql = s.length();
  for (int q = 0; q < ql ; q++) {
      if (s[q] == '0') { lcd.setCursor(x+q, y) ; lcd.write(0); lcd.setCursor(x+q, y+1); lcd.write(4); }
      if (s[q] == '1') { lcd.setCursor(x+q, y) ; lcd.write(1); lcd.setCursor(x+q, y+1); lcd.write(1); }
      if (s[q] == '2') { lcd.setCursor(x+q, y) ; lcd.write(2); lcd.setCursor(x+q, y+1); lcd.write(5); }
      if (s[q] == '3') { lcd.setCursor(x+q, y) ; lcd.write(3); lcd.setCursor(x+q, y+1); lcd.write(7); }
      if (s[q] == '4') { lcd.setCursor(x+q, y) ; lcd.write(4); lcd.setCursor(x+q, y+1); lcd.write(1); }
      if (s[q] == '5') { lcd.setCursor(x+q, y) ; lcd.write(5); lcd.setCursor(x+q, y+1); lcd.write(7); }
      if (s[q] == '6') { lcd.setCursor(x+q, y) ; lcd.write(5); lcd.setCursor(x+q, y+1); lcd.write(4); }
      if (s[q] == '7') { lcd.setCursor(x+q, y) ; lcd.write(2); lcd.setCursor(x+q, y+1); lcd.write(1); }
      if (s[q] == '8') { lcd.setCursor(x+q, y) ; lcd.write(6); lcd.setCursor(x+q, y+1); lcd.write(4); }
      if (s[q] == '9') { lcd.setCursor(x+q, y) ; lcd.write(6); lcd.setCursor(x+q, y+1); lcd.write(7); }
      if (s[q] == ' ') { lcd.setCursor(x+q, y) ; lcd.print(" "); lcd.setCursor(x+q, y+1); lcd.print(" "); }
      if (s[q] == ':') { lcd.setCursor(x+q, y) ; lcd.write(165); lcd.setCursor(x+q, y+1); lcd.write(165); }
  }
}
