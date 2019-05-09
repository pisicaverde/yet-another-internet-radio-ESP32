

int usedBuffer() {
  // the function returns the number of bytes in use in circular buffer
  return(( DATA_BUFFER_SIZE + writePointer - readPointer ) % DATA_BUFFER_SIZE); 
  }


void updateRtc() {
  if(!getLocalTime(&rtc)){ Serial.println("[updateRtc] Failed to obtain time"); return; } // this function is called frequently, so there is serial output only on errors
  rtc_y  = rtc.tm_year + 1900;
  rtc_m  = rtc.tm_mon + 1;
  rtc_d  = rtc.tm_mday;
  rtc_h  = rtc.tm_hour;
  rtc_mn = rtc.tm_min;
  rtc_s  = rtc.tm_sec;
  rtc_dw = rtc.tm_wday + 0;
  temp_celsius = ( temprature_sens_read() - 32 ) / 1.8;
}


void die() { 
  // TO DO: this should be replaced with a more elegant solution
  Serial.println(F("[die] DEAD BY AN ERROR")); while (1) {yield();} 
  } 
