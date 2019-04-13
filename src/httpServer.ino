
void handleRoot() {
  String r = "";
  float d = millis();
  Serial.print("[handleRoot] Page requested by: ");
  Serial.print(server.header("User-Agent:"));
  Serial.println(server.header("Host:"));

  if(!SPIFFS.begin(true)){ Serial.println("[handleRoot] ERROR ACCESING SPIFFS"); }
  File file;

  String tabName = server.arg(0); if (tabName == "") tabName = "stat";
  
  if (tabName == "stat") {     
      if (server.arg(1) == "clock") { Serial.println("[handleRoot] --> CLOCK"); fgApp = 0; fgAppPrev = 100; r = "Switched to clock"; }
      if (server.arg(1) == "play")  { Serial.println("[handleRoot] --> PLAY");  fgApp = 1; fgAppPrev = 100; r = "Switched to internet radio"; }
      if (server.arg(1) == "like")  { Serial.println("[handleRoot] --> LIKED SONG"); likedSong(); r = "Saved to Liked Songs"; }
      if (server.arg(1) == "prev")  { Serial.println("[handleRoot] --> PREV"); if (stationNow >= 1)           { Serial.println(" OK"); isClosing = 1; stopDisconnect(); stationNow--; fgAppPrev = 0; isClosing = 0; }  }
      if (server.arg(1) == "next")  { Serial.println("[handleRoot] --> NEXT"); if (stationNow < stationCnt-1) { Serial.println(" OK"); isClosing = 1; stopDisconnect(); stationNow++; fgAppPrev = 0; isClosing = 0; }  }
  }
  
  if (tabName == "pls") {     
      if (server.argName(1) == "butt1")  { isClosing = 1; stopDisconnect(); stationNow = server.arg(1).toInt(); fgAppPrev = 0; fgApp = 1; isClosing = 0; }
      
      if (server.argName(1) == "butt2")  {   // delete a preset
                                             byte rem = server.arg(1).toInt(); 
                                             if (stationNow >= rem) stationNow--; // move pointer up
                                             Serial.printf("--> Should kill station # %u\r\n", rem); 
                                             for(byte i = rem; i < stationCnt - 1; i++) stationList[i] = stationList[i + 1];
                                             stationCnt--;
                                             jsonSave();
                                          }
      if (server.argName(1) == "butt3")  {   // move up
                                            byte r = server.arg(1).toInt(); // this one will be moved up 1 position
                                            if (stationNow == r) { stationNow = r - 1; } // if we moved the preset currently playing (move pointer up)
                                                  else if (stationNow == r - 1) stationNow = r ;
                                            stations stationTmp = stationList[r - 1]; // making a copy
                                            stationList[r-1] = stationList[r];
                                            stationList[r] = stationTmp;
                                            jsonSave();
                                          }                                   
      }

  
  if (tabName == "add") {  
      
      stationList[stationCnt].title = String(server.arg(1));
      stationList[stationCnt].host  = String(server.arg(2));
      stationList[stationCnt].port  = server.arg(3).toInt();
      stationList[stationCnt].path  = String(server.arg(4));
      jsonSave();
      stationCnt++;
  }
  
  if (tabName == "audio") {  
        VS_vol_l      = server.arg(1).toInt();
        VS_vol_r      = server.arg(2).toInt();
        ST_AMPLITUDE  = server.arg(3).toInt();
        ST_FREQLIMIT  = server.arg(4).toInt();
        SB_AMPLITUDE  = server.arg(5).toInt();
        SB_FREQLIMIT  = server.arg(6).toInt();
        vsPlayer.sciWrite(VS1053_REG_VOLUME, VS_vol_l * 256 + VS_vol_r);  delay(10); // set startup vol & bass/treble adj
        vsPlayer.sciWrite(VS1053_REG_BASS, SB_FREQLIMIT + SB_AMPLITUDE * 16 + ST_FREQLIMIT * 256 + ST_AMPLITUDE * 4096); delay(10);
        jsonSave(); 
  }

  if (tabName == "wifi") {
        server.arg(1).toCharArray(user_agent, 20);
        server.arg(2).toCharArray(ap_pass, 20);
        server.arg(3).toCharArray(wifi0_ssid, 20);
        server.arg(4).toCharArray(wifi0_pass, 20);
        server.arg(5).toCharArray(wifi1_ssid, 20);
        server.arg(6).toCharArray(wifi1_pass, 20);
        server.arg(7).toCharArray(wifi2_ssid, 20);
        server.arg(8).toCharArray(wifi2_pass, 20);
        jsonSave(); 
  }

  if (tabName == "wifi2") {
        if (server.arg(1) == "RESTART") { r = "Restarting. You should not see this message."; ESP.restart();}
  }

  if (tabName == "liked") {
        if (server.arg(1) == "Delete all") {
        SPIFFS.remove("/songlist.txt");
        r = "Deleted";
        }
  }

  if (tabName == "tmr") { 
      byte tmpDoW = 0;
      for (byte argNo = 0; argNo < server.args(); argNo++) {
        for (byte dayNo = 0; dayNo < 7; dayNo++) {
          String str = "day" + String(dayNo);
          if (str == server.argName(argNo + 3)) { /* Serial.print(str); Serial.print(":"); Serial.println(server.arg(argNo+3)); */ tmpDoW += server.arg(argNo+3).toInt(); }
        }
      }
      autoDoW = tmpDoW;

      autoStartH = server.arg(1).substring(0,2).toInt();
      autoStartM = server.arg(1).substring(3,5).toInt();
      autoStopH = server.arg(2).substring(0,2).toInt();
      autoStopM = server.arg(2).substring(3,5).toInt();  
      Serial.printf("[handleRoot] autoStartH=%u autoStartM=%u autoStopH=%u autoStopM=%u\r\n", autoStartH, autoStartM, autoStopH, autoStopM);
      Serial.print("[handleRoot] new autoDow="); Serial.println(autoDoW);
      jsonSave();
  }
  

  String m ;

  m = "<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
<meta charset=\"utf-8\">\
<title>" + String(user_agent) + "</title>\
<style type=\"text/css\">\
html,body{font-family:\"Segoe UI\",Roboto,Ubuntu,\"Helvetica Neue\",Helvetica,sans-serif;font-size:0.95em;width:auto;height:100%;}\
.tab{overflow:hidden;border:1px solid #ccc;background-color:#f1f1f1;} \
.tab button{background-color:inherit;float:left;outline:none;padding:10px 25px; margin:10px 0px 10px 10px;border-radius:5px;border:1px solid gray;box-shadow:0 4px 2px -2px gray;transition:0.3s;}\
.tab button:hover{background-color:#ddd;}\
.tab button.active {background-color:#ccc;font-weight:bold;transform:translateY(4px);}\
.tabcontent{display:none;padding:6px 12px;border:1px solid #ccc;border-top:none;animation:fadeEffect 1s;}\
pre{overflow-x:auto;white-space:pre-wrap;white-space:-moz-pre-wrap;white-space:-pre-wrap;white-space:-o-pre-wrap;word-wrap:break-word;}\
@keyframes fadeEffect{from{opacity:0;}to{opacity:1;}}\
button{outline:none;padding:10px 15px;margin:2px;border:1px solid gray;box-shadow:0 4px 4px -1px gray;font-weight:bold;}\
button:hover{transform:translateY(4px);box-shadow:0 2px 2px -1px gray;}\
.s1{padding:2px 8px;font-weight:normal;}\
dt{width:150px;float:left;}\
fieldset{border:1px solid gray;border-radius:5px;padding-bottom:12px;margin-bottom:10px;} \
legend{font-weight:bold;}\
label:hover{font-weight:bold;}\
input[type=text],input[type=number]{border:1px solid gray;width:120px;padding:5px 5px;margin:8px 0;box-shadow:inset 0 4px 4px -1px gray;margin:0px 8px;}\
footer{font-size:smaller;margin-top:10px;}\
#elem{animation:3s ease 0s normal forwards 1 fadein;opacity:1}\
@keyframes fadein{0%{opacity:1} 80%{opacity:1} 100%{opacity:0}}\
</style>\
<script type=\"text/javascript\">\
function openTab(evt, tabName) {\
var i,tabcontent,tablinks;\
tabcontent=document.getElementsByClassName(\"tabcontent\");\
for(i=0;i<tabcontent.length; i++){tabcontent[i].style.display=\"none\";}\
tablinks=document.getElementsByClassName(\"tablinks\");\
for(i=0;i<tablinks.length;i++){tablinks[i].className=tablinks[i].className.replace(\" active\",\"\");}\
document.getElementById(tabName).style.display=\"block\";evt.currentTarget.className+=\" active\"; }\
</script>\
</head>\
<body>";



// uncomment for displaying http info in page
/*
m += "<pre>URI: " + String(server.uri()) + "\nMethod: " + String((server.method() == HTTP_GET) ? "GET" : "POST") + "\nArguments: " + String(server.args()) + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    m += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  m += "</pre>";
*/




m += "<h1 style=\"float:left\"><a href=\"/\">" + String(user_agent) + "</a></h1>";

if (r != "") m += "<div id=elem style=\"float:left; width: 300px;background-color:#e3f7fc;color:#555;border:.1em solid;border-color:#8ed9f6;padding:10px 10px 10px 10px;margin:10px;border-radius:10px;\">" + r + "</div>";

m +="<div style=\"clear:both;\"></div><div class=\"tab\">\
  <button class=\"tablinks\" onclick=\"openTab(event, 'stat')\" "  ; if (tabName == "stat")  m += "id=\"defaultOpen\" "; m += ">Status</button>\
  <button class=\"tablinks\" onclick=\"openTab(event, 'pls')\" "   ; if ((tabName == "pls") || (tabName == "add"))   m += "id=\"defaultOpen\" "; m += ">Stations</button>\
  <button class=\"tablinks\" onclick=\"openTab(event, 'audio')\" " ; if (tabName == "audio") m += "id=\"defaultOpen\" "; m += ">Audio</button>\
  <button class=\"tablinks\" onclick=\"openTab(event, 'wifi')\" "  ; if (tabName == "wifi")  m += "id=\"defaultOpen\" "; m += ">WiFi</button>\
  <button class=\"tablinks\" onclick=\"openTab(event, 'liked')\" " ; if (tabName == "liked") m += "id=\"defaultOpen\" "; m += ">Liked</button>\
  <button class=\"tablinks\" onclick=\"openTab(event, 'tmr')\" "   ; if (tabName == "tmr")   m += "id=\"defaultOpen\" "; m += ">Schedule</button>\
</div>\
<div id=\"stat\" class=\"tabcontent\">";

m += "<dl>";
m += "<dt>Selected station:</dt><dd><strong>"; 
m += (String)(stationNow) + ") " + stationList[stationNow].title + " " + stationList[stationNow].host + ":" + stationList[stationNow].port + stationList[stationNow].path + "</strong></dd>";
m += "<dt>Foreground app:</dt>";
  switch(fgApp) {
    case 0:  m += "<dd>CLOCK</dd>"; break;
    case 1:  m += "<dd>RADIO (PLAYING)</dd>"; break;
    case 2:  m += "<dd>LIST</dd>"; break;
    case 3:  m += "<dd>SETUP</dd>"; break;
    default: m += "<dd>UNKNOWN / OTHER</dd>"; break;
  }
if (fgApp == 1) { m += "<dt>Advertised as:</dt><dd>" + metaStationName + "</dd><dt>Bitrate:</dt><dd>" + metaBR + " kbps</dd><dt>Now playing:</dt><dd><strong>" + metaDataTxt + "</strong></dd>"; }

m += "</dl><div style=\"clear:both;\"></div><form action=\"/\" method=\"POST\">\
<input type=hidden name=formname value=stat>\
<button type=submit name=butt1 value=\"prev\" " ; if (stationNow == 0) m += "disabled"; m += ">PREV</button>\
<button type=submit name=butt1 value=\"play\" " ; if (fgApp == 1) m += "disabled"; m += ">PLAY</button>\
<button type=submit name=butt1 value=\"clock\" "; if (fgApp != 1) m += "disabled"; m += ">PAUSE</button>\
<button type=submit name=butt1 value=\"next\" " ; if (stationNow == stationCnt-1) m += "disabled"; m += ">NEXT</button>\
<button type=submit name=butt1 value=\"like\" " ; if (fgApp != 1) m += "disabled"; m += ">LIKE</button>\
</form><div style=\"clear:both;\"></div>";

m += "</div><div id=\"pls\" class=\"tabcontent\"><form action=\"\" method=\"POST\"><fieldset><legend>Add presets:</legend><input type=hidden name=formname value=add>\
<input type=text name=givenname placeholder=\"preset title\"> \
http://<input type=text name=newhost id=newhost placeholder=\"www.clasicradio.ro\">:\
<input type=number name=newport id=newport placeholder=\"8000\">\
<input type=text name=newpath id=newpath placeholder=\"/stream4\">\
<button type=button onClick=\"javascript:window.open('http://' + document.getElementById('newhost').value+':'+document.getElementById('newport').value+document.getElementById('newpath').value, '_blank',\
'width=500, height=300');\">Test</button>\
<button type=submit name=butt1>Save</button></fieldset></form>"; 

m += "<form action=\"/\" method=\"POST\"><input type=hidden name=formname value=pls>";
 for (byte tmp = 0; tmp < stationCnt ; tmp++) {
    m += "<button class=s1 type=submit name=butt1 value=\"" + String(tmp) + "\""; if ((tmp == stationNow) && (fgApp == 1)) m+= " disabled" ; m += ">Play</button>";
    m += "<button class=s1 type=button onClick=\"javascript:window.open('http://" + stationList[tmp].host + ":" + stationList[tmp].port + stationList[tmp].path + "','_blank','width=500,height=300');\">Test</button>";
    m += "<button class=s1 type=submit name=butt2 value=\"" + String(tmp) + "\" onClick=\"return confirm('Do you really want to delete preset?');\">Del</button>";
    m += "<button class=s1 type=submit name=butt3 value=\"" + String(tmp) + "\" "; if (tmp == 0) m += " disabled" ; m += ">Up</button>";
    if (tmp == stationNow) m += "<strong>";
    m += (String)(tmp) + ") " + stationList[tmp].title + " " + stationList[tmp].host + ":" + stationList[tmp].port + stationList[tmp].path ;
    if (tmp == stationNow) m += "</strong>";
    m += "<br>";
  }

m += "</form></div>\
<div id=\"audio\" class=\"tabcontent\"><form action=\"/\" method=\"POST\"><input type=\"hidden\" name=\"formname\" value=\"audio\">\
<fieldset style=\"float:left; width: 280px;\"><legend>Volume</legend>\
L min<input name=\"VS_vol_l\" type=\"range\" min=\"0\" max=\"255\" value=\"" + String(VS_vol_l) + "\" style=\"width: 200px; direction: rtl;\">max<p>\
R min<input name=\"VS_vol_r\" type=\"range\" min=\"0\" max=\"255\" value=\"" + String(VS_vol_r) + "\" style=\"width: 200px; direction: rtl;\">max</fieldset>\
<fieldset style=\"float:left; width: 170px;\"><legend>Treble control</legend>\
min <input name=\"ST_AMPLITUDE\" id=\"vol\" type=\"range\" min=\"0\" max=\"15\" value=\"" + String(ST_AMPLITUDE)+ "\" style=\"width: 80px;\">max<p>\
1 kHz<input name=\"ST_FREQLIMIT\" id=\"vol\" type=\"range\" min=\"1\" max=\"15\" value=\"" + String(ST_FREQLIMIT)+ "\" style=\"width: 80px;\">15 kHz</fieldset>\
<fieldset style=\"float:left; width: 170px;\"><legend>Bass enhancement</legend>\
min <input name=\"SB_AMPLITUDE\" id=\"vol\" type=\"range\" min=\"0\" max=\"15\" value=\"" + String(SB_AMPLITUDE)+ "\" style=\"width: 80px;\">max<p>\
0 Hz <input name=\"SB_FREQLIMIT\" id=\"vol\" type=\"range\" min=\"1\" max=\"15\" value=\"" + String(SB_FREQLIMIT)+ "\" style=\"width: 80px;\">1500 Hz</fieldset>\
<button type=submit name=butt1>Apply</button></form><div style=\"clear:both;\"></div></div>\
\
<div id=\"wifi\" class=\"tabcontent\"><p>";

m +="<form action=\"\" method=\"POST\"><input type=\"hidden\" name=\"formname\" value=\"wifi\"><fieldset style=\"float:left; width: 380px;\"><legend>Info</legend><dl>";

if (softApActive == 1) { m += "<dt>Mode:</dt><dd>SoftAP, " + String(nrConns) + "connected clients.</dd><dt>AP:</dt><dd>" + user_agent + " , pass: " + ap_pass + "</dd><dt>IP:</dt><dd>" + (WiFi.softAPIP()) + "</dd>";}
                  else { m += "<dt>Mode:</dt><dd>STA mode</dd><dt>AP:</dt><dd>" + WiFi.SSID() + "</dd><dt>IP:</dt><dd>"; IPAddress ip = WiFi.localIP();  m += String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]) + "</dd>"; 
                         m += "<dt>RSSI:</dt><dd>" + String(WiFi.RSSI()) + " dBm</dd>\
                               <small>-30 dBm: AMAZING! max achivable<br>-67 dBm: Very good, suitable for streaming<br>-70 dBm: Okay, minimum signal strenth for reliable delivery<br>\
                               -80, -90 dBm: not good / unusable</small>";
                        }

m += "</dl></fieldset><fieldset style=\"float:left; width: 18%;\"><legend>Device name</legend>Max 20 chars for each field.<p>\
<input type=text name=user_agent value=\"" + String(user_agent) + "\">Device name<small><br>(User agent, host name, AP SSID)</small><p>\
<input type=text name=ap_pass value=\"" + ap_pass + "\">Password <small><br>(for softAP mode)</small></fieldset><div style=\"clear:both;\"></div>\
<fieldset style=\"float:left; width: 15%;\"><legend>AP1</legend><input type=text name=wifi0_ssid value=\"" + wifi0_ssid + "\">name<p><input type=text name=wifi0_pass value=\"" + wifi0_pass + "\">pass</fieldset>\
<fieldset style=\"float:left; width: 15%;\"><legend>AP2</legend><input type=text name=wifi1_ssid value=\"" + wifi1_ssid + "\">name<p><input type=text name=wifi1_pass value=\"" + wifi1_pass + "\">pass</fieldset>\
<fieldset style=\"float:left; width: 15%;\"><legend>AP3</legend><input type=text name=wifi2_ssid value=\"" + wifi2_ssid + "\">name<p><input type=text name=wifi2_pass value=\"" + wifi2_pass + "\">pass</fieldset>\
<div style=\"clear:both;\"></div><button type=\"submit\">Save</button></form>\
<form action=\"/\" method=\"POST\"><input type=\"hidden\" name=\"formname\" value=\"wifi2\"><button type=\"submit\" name=butt1>Restart to apply changes</button></form></div>\
<div id=\"liked\" class=\"tabcontent\"><form action=\"/\" method=\"POST\" onsubmit=\"return confirm('Do you really want to delete all liked tracks?');\">\
<fieldset><legend>Liked tracks:</legend>\
<input type=\"hidden\" name=\"formname\" value=\"liked\"><pre>";

    file = SPIFFS.open("/songlist.txt", "r"); if(!file){ m += "[handleRoot] OPENING LIST FAILED"; }
    while(file.available()) { char c= file.read(); m += c ; }
    file.close();
m += "</pre><button type=\"submit\" name=\"butt1\" value=\"Delete all\">Delete All</button></fieldset></form></div>\
<div id=\"tmr\" class=\"tabcontent\"><form action=\"/\" method=\"POST\"><input type=hidden name=formname value=tmr><fieldset><legend>Scheduler:</legend>\
<p>Select days and timeframe. Unselect all days to disable scheduling. Schedule has priority over manual orders and cannot be overriden.</p>\
<p>At <input type=\"time\" name=\"autoStart\" value=\""; m += (autoStartH <= 9)?("0"):(""); m += String(autoStartH) + ":"; 

m += (autoStartM <= 9)?("0"):(""); m += String(autoStartM) + "\"> device will switch to fgApp=1.</p>\
<p>At <input type=\"time\" name=\"autoStop\" value=\"" ; m += (autoStopH  <= 9)?("0"):(""); m += String(autoStopH)  + ":"; 

m += (autoStopM <= 9)?("0"):(""); m += String(autoStopM) +  "\"> device will switch to fgApp=0.</p>\
<input type=\"checkbox\" name=\"day0\" value=\"1\"" ; if (bitRead(autoDoW, 0)) { m+= "1\" checked "; } ; m+= ">Sunday<br>\
<input type=\"checkbox\" name=\"day1\" value=\"2\"" ; if (bitRead(autoDoW, 1)) { m+= "2\" checked "; } ; m+= ">Monday<br>\
<input type=\"checkbox\" name=\"day2\" value=\"4\"" ; if (bitRead(autoDoW, 2)) { m+= "4\" checked "; } ; m+= ">Tuesday<br>\
<input type=\"checkbox\" name=\"day3\" value=\"8\"" ; if (bitRead(autoDoW, 3)) { m+= "8\" checked "; } ; m+= ">Wednesday<br>\
<input type=\"checkbox\" name=\"day4\" value=\"16\"" ; if (bitRead(autoDoW, 4)) { m+= "16\" checked "; } ; m+= ">Thursday<br>\
<input type=\"checkbox\" name=\"day5\" value=\"32\"" ; if (bitRead(autoDoW, 5)) { m+= "32\" checked "; } ; m+= ">Friday<br>\
<input type=\"checkbox\" name=\"day6\" value=\"64\"" ; if (bitRead(autoDoW, 6)) { m+= "64\" checked "; } ; m+= ">Saturday<br>\
<button type=submit name=butt1>Save</button></fieldset></form></div><script>document.getElementById(\"defaultOpen\").click();</script>";

  d = millis() - d;

  if(!getLocalTime(&rtc)){ Serial.println("[updateRtc] Failed to obtain time"); return; }
  rtc_y  = rtc.tm_year + 1900;
  rtc_m  = rtc.tm_mon + 1;
  rtc_d  = rtc.tm_mday;
  rtc_h  = rtc.tm_hour;
  rtc_mn = rtc.tm_min;
  rtc_s  = rtc.tm_sec;
  rtc_dw = rtc.tm_wday + 0;
  temp_celsius = ( temprature_sens_read() - 32 ) / 1.8;

  //rssi
  int db = WiFi.RSSI(); 
  int q;
  if (db >= -50) { q = 100; } else if (db <= -100) { q = 0 ;}
  else {q = 2 * ( WiFi.RSSI() + 100) ; } // q este RSSI in procente

  //updateRtc();
  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &rtc);
  
  m += "<br><small>" + String(timeStringBuff) + " and " + String(temp_celsius) + "&deg;C. Doing shit since " + String(millis()) + " milliseconds. Firmware upload: " + __DATE__ + " " + __TIME__ ;
  m += ". WiFi level: " + String(q) + "%. Buffer: " + String(map(usedBuffer(), 0, DATA_BUFFER_SIZE, 0, 99), DEC) + "%. Page rendered in " + String(d/1000) + " s</small></body></html>";
  server.send(200, "text/html", m);

}

