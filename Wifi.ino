
const short Err_Ok   = 0;
const short Err_Err  = 1;
const short Cmd_Te1  = 0; // temperature 1
const short Cmd_Te2  = 1; // temperature 2
const short Cmd_Prs  = 2; // pressure
const short Cmd_Rst  = 3; // reset max min
const short Cmd_Sta  = 4; // status
const short Cmd_None = 5;
const short Msg_Ok   = 0;
const short Msg_None = 1;
const short Msg_Err  = 2;

long pres_min, pres_cur, pres_max;   // 0.01 hPa
short temp1_min, temp1_cur, temp1_max; // 0.1 deg C
short temp2_min, temp2_cur, temp2_max; // 0.1 deg C
long last_check_time;
long last_print_time;
int  no_restarts;

void setup() {
  Serial.begin(9600);
  Esp_Start();
  Bmp_Start();
  Serial.println(F("Arduino started..."));
  Esp_StartTcpServer();  
  last_check_time = millis();
  last_print_time = last_check_time;
  no_restarts = 0;
  pres_cur = 100000;
  temp1_cur = 200;
  temp2_cur = 200;
  reset_pres(&pres_min, &pres_cur, &pres_max);
  reset_temp(&temp1_min, &temp1_cur, &temp1_max);
  reset_temp(&temp2_min, &temp2_cur, &temp2_max);
}

void loop() {
  short res;
  unsigned long cur_time = millis();  
  res = Cmd_CheckCmd();
  switch (res) {
    case Cmd_Te1:
      Cmd_Respond_Tmp(temp1_min,temp1_cur,temp1_max);
    break;
    case Cmd_Te2:
      Cmd_Respond_Tmp(temp2_min,temp2_cur,temp2_max);
    break;
    case Cmd_Prs:
      Cmd_Respond_Prs(pres_min,pres_cur,pres_max);
    break;
    case Cmd_Rst:
      reset_pres(&pres_min, &pres_cur, &pres_max);
      reset_temp(&temp1_min, &temp1_cur, &temp1_max);
      reset_temp(&temp2_min, &temp2_cur, &temp2_max);
    break;
    case Cmd_Sta:
      Cmd_Respond_Sta(no_restarts,cur_time);
    break;
    case Cmd_None:
    break;
  }
  
  if ((cur_time-last_print_time) > 10000) {    
    last_print_time = cur_time;
    Bmp_Read(&temp1_cur,&pres_cur);          
    if (pres_cur < pres_min)
      pres_min = pres_cur;
    if (pres_cur > pres_max)
      pres_max = pres_cur;
    if (temp1_cur < temp1_min)
      temp1_min = temp1_cur;
    if (temp1_cur > temp1_max)
      temp1_max = temp1_cur;    
    Serial.print("T =");
    Serial.print(cur_time);
    Serial.print(" (");
    Serial.print(no_restarts);
    Serial.println(") > ");
  }
  if ((cur_time-last_check_time) > 60000) {
    last_check_time = cur_time;
    if (Grd_CheckConnection() == Err_Err) {
      Serial.println(F("Restarting..."));
      no_restarts++;
      Esp_StartTcpServer();
    }
  }
  delay(100);
}

void reset_temp(short *temp_min, short *temp_cur, short *temp_max) {
  *temp_min = *temp_cur;
  *temp_max = *temp_cur;
}

void reset_pres(long *pres_min, long *pres_cur, long *pres_max) {
  *pres_min = *pres_cur;
  *pres_max = *pres_cur;
}

/*
 void loop() { 
  
  // I. Handle incoming commands
  {
    bool error = false;
    int chn, cmd;
    
    Esp_ReadMessage(100,"\r\n");
    if (Esp_MsgFound()) {           
      Esp_PrintMessage(true);    
      if (Esp_ParseMessage(&chn,&cmd)) {
        if (cmd == 1) {   
          // get min, current and max                             
        }
        else
          if (cmd == 2) {
            // reset min and max to cur
            temp_max = temp_cur; temp_min = temp_cur;
            pres_max = pres_cur; pres_min = pres_cur;
          }
        int len = 3 * 5 + 3 * 6 + 1;
        char rsp[len];            
        temp_serialise(rsp,0,temp_min);
        temp_serialise(rsp,5,temp_cur);
        temp_serialise(rsp,10,temp_max);              
        pres_serialise(rsp,15,pres_min);
        pres_serialise(rsp,21,pres_cur);
        pres_serialise(rsp,27,pres_max);
        rsp[len-1] = '\0';
        Esp_ReadMessage(100,"\r\n"); Esp_PrintMessage(true);
        Esp_SendPrep(strlen(rsp),chn);
        Esp_ReadMessage(100,"> "); Esp_PrintMessage(true);
        Esp_Send(rsp);        
        Esp_ReadMessage(500,"OK\r\n"); Esp_PrintMessage(true);
        error = !Esp_TermStringFound();        
        if (!error) {
          Esp_ReadMessage(500,"CLOSED\r\n"); Esp_PrintMessage(true);  
          error = !Esp_TermStringFound();
        }
      }      
    }
    if (error)
      Esp_TcpServer();
    else      
      if (Serial.available()) {
        int d = Serial.read();
        if (d == '!') {
          Esp_TcpServer();   
          // read trailing CR LF
          while (Serial.available()) 
            Serial.read();
        }
        else
          Esp_Write(d);
      }
  }

  // II. Read sensors
  {
    short temp;    
    if (ctr1 == 64) {      
      Bmp_Read(&temp,&pres_cur);      
      temp_cur = temp * 10;
      if (pres_cur < pres_min)
        pres_min = pres_cur;
      if (pres_cur > pres_max)
        pres_max = pres_cur;
      if (temp_cur < temp_min)
        temp_min = temp_cur;
      if (temp_cur > temp_max)
        temp_max = temp_cur;
      ctr1 = 0;
      ctr2++;
      if (ctr2 == 2)
        Esp_Send("AT+CIFSR");
      else if (ctr2 == 4) {
        Esp_Send("AT+CIPSTATUS");
        ctr2 = 0;
      }    
    }
    else
      ctr1++;
  }

}
*/


