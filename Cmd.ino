short chn; 

short Cmd_CheckCmd() {
  unsigned long tmax = 100; // ms
  unsigned long t_start = millis();
  short res = Msg_None;
  while ((millis() < (t_start + tmax)) && (res == Msg_None)) {
    res = Esp_ReadMessage("\r\n",100); // read a CRLF terminated message
    if (res == Msg_Ok) {
      // message found, check if it contains "+IPD:"
      if (strstr(Esp_Str(),"+IPD") ==  '\0') {                
        Esp_ConsumeMessage(res);
        res = Msg_None; // continue reading messages
      }       
    }    
  }
  if (res == Msg_Ok) {
    // parse message
    short cmd = Cmd_None;       
    if (strstr(Esp_Str(),"te1") != '\0') 
      cmd = Cmd_Te1;
    else
      if (strstr(Esp_Str(),"te2") != '\0')
        cmd = Cmd_Te2; 
      else 
        if (strstr(Esp_Str(),"prs") != '\0')
          cmd = Cmd_Prs;
        else 
          if (strstr(Esp_Str(),"rst") != '\0')
            cmd = Cmd_Rst;
          else
            if (strstr(Esp_Str(),"sta") != '\0')
              cmd = Cmd_Sta;             
    const char *cp = Esp_Str();
    chn = *(cp+5)-'0'; // assume this command is handled before next call to Cmd_CheckCmd
    Esp_ConsumeMessage(res);
    return cmd;  
  }
  else
    return Cmd_None;  
}

short Cmd_Respond_Tmp(int min, int cur, int max) {
  // 3*5 chars chars temperature 0.1 deg C
  short len = 3 * 5 + 1;
  char rsp[len];
  sprintf(&rsp[0],"%5d",min);
  sprintf(&rsp[5],"%5d",cur);  
  sprintf(&rsp[10],"%5d",max);  
  short res = Cmd_Respond(rsp);
  return res;    
}

short Cmd_Respond_Prs(long min, long cur, long max) {
  // 3*6 chars chars pressure 0.01 hPa
  short len = 3 * 6 + 1;
  char rsp[len];
  sprintf(&rsp[0],"%6ld",min);
  sprintf(&rsp[6],"%6ld",cur);  
  sprintf(&rsp[12],"%6ld",max);  
  short res = Cmd_Respond(rsp);
  return res;  
}

short Cmd_Respond_Sta(int no_restarts, long time) {
  //  3 chars no_restarts
  // 10 chars time
  short len = 3 + 10 + 1;
  char rsp[len];
  sprintf(&rsp[0],"%3d",no_restarts);
  sprintf(&rsp[3],"%10ld",time);  
  short res = Cmd_Respond(rsp);
  return res;
}


short Cmd_Respond(char *rsp) {
  short res;
  res = Esp_ReadMessage("\r\n",10); // read a CRLF terminated message
  if (res == Msg_Ok)
    Esp_ConsumeMessage(res); // only in case cmd comes from putty
  //else 
  //  return Err_Err;
  char msg[20];
  short len = strlen(rsp);
  sprintf(msg,"AT+CIPSEND=%d,%d",chn,len + 2);  
  Esp_SendMessage(msg);
  res = Esp_ReadMessage("> ",100);
  Esp_ConsumeMessage(res);
  if (res == Msg_Ok)
    Esp_SendMessage(rsp);
  else
    return Err_Err;

  res = Esp_ReadMessage("SEND OK\r\n",2000);
  Esp_ConsumeMessage(res);
  if (res != Msg_Ok)
    return Err_Err;

  //res = Esp_ReadMessage("CLOSED\r\n",100);
  //if (res != Msg_Ok)
  //  return Err_Err;
    
  return Err_Ok;  
}

/*
void rst_serialise(char *rsp, int ptr, int rst) {
  // 3 char restart count
  int len = 3;
  for (int i = 0; i < len; i++) {
    int digit = rst % 10;
    rsp[ptr + len - i - 1] = digit + '0';
    rst = rst / 10;
  }
}

void pres_serialise(char *rsp, int ptr, long pres) {
  // 6 char air pressure: 10^4,...,10^-2 Pa
  int len = 6;
  for (int i = 0; i < len; i++) {
    int digit = pres % 10;
    rsp[ptr + len - i - 1] = digit + '0';
    pres = pres / 10;
  }
}

void temp_serialise(char *rsp, int ptr, int temp) {
  // 5 char temperature: sign, 10^1,...,10^-2 C  
  if (temp < 0) {
    rsp[ptr] = '-';
    temp = -temp;
  }
  else
    rsp[ptr] = '+';       
  int len = 5;
  for (int i = 1; i < len; i++) {
    int digit = temp % 10;
    rsp[ptr + len - i] = digit + '0';
    temp = temp / 10;
  }   
}

*/
