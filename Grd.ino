short Grd_CheckConnection() {
  boolean err = false;
  short res;
  
  Esp_Flush();    
  Esp_SendMessage("AT+CIPSTATUS"); 
  res = Esp_ReadMessage("OK\r\n",1000); Esp_ConsumeMessage(res);      
  err = err || (res != Err_Ok);  
  if (err) 
    return Err_Err;
  else
    return Err_Ok;
}

