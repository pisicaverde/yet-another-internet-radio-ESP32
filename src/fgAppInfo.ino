void fgAppInfo() {
    // shows connection info on lcd
    // TO DO: on key press exit SoftAP and enter STA

    // if we're coming from internet radio, disconnect, stop, clean
    if (fgAppPrev != fgApp) { Serial.println(F("[fgAppInfo] Showing infos")); 
                              fgAppPrev = fgApp; 
                              jsonSave();
                              lcd.clear() ; lcd.setCursor(0,0); lcd.print("3-INFO");     
                              stopDisconnect();
                            }    
                          
    if (softApActive == 1) { lcd.setCursor(0,1);
                             lcd.print(" SoftAP ("); 
                             lcd.print(nrConns); lcd.print(")");
                             lcd.setCursor(0,2); lcd.print("AP:\""); lcd.print(user_agent); lcd.print("\"/"); lcd.print(ap_pass);
                             lcd.setCursor(0,3); lcd.print("IP:"); lcd.print(WiFi.softAPIP());
                           }
            
                    else {  lcd.setCursor(0,1);
                            lcd.print("STATION mode");
                            lcd.setCursor(0,2); lcd.print("AP:"); lcd.print(WiFi.SSID()); 
                            lcd.setCursor(0,3); lcd.print("IP:"); IPAddress ip = WiFi.localIP();  lcd.print(ip); 
                         }

    delay(500);
                             
    keyReady = 1;
}

