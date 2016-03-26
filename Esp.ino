#include <SoftwareSerial.h>
const byte rxPin = 11;
const byte txPin = 10;
const byte rstPin = 2;
SoftwareSerial esp(rxPin, txPin);

// buffer for incoming messages
const int esp_ptr_mx = 127;
int esp_ptr = 0; // free place in buffer 
char esp_buf[esp_ptr_mx+1];

// connect Rx of ESP to pin 10
// connect Tx of ESP to pin 11

void Esp_Start() {
  pinMode(rxPin,INPUT);
  pinMode(txPin,OUTPUT);
  pinMode(rstPin,OUTPUT);
  digitalWrite(rstPin,HIGH);  
  esp.begin(9600);
}

const char *Esp_Str() {
  return &esp_buf[0];  
}

void Esp_SendMessage(const char *msg) {
  esp.write(msg);  
  esp.write("\r\n");
  Serial.print("S > ");
  Serial.println(msg);
}

void Esp_ConsumeMessage(short res) {
  Serial.print("R");
  Serial.print(res);
  Serial.print("> ");
  Serial.println(esp_buf);
  esp_ptr = 0;
}

short Esp_ReadMessage(const char *termStr, int tmax) {
    // read message into esp_buf (zero terminated)
    // reading continues for maximum of tmax ms
    // reading terminates on :
    //   termStr  --> Msg_Ok is returned
    //   timeout  --> Msg_None is returned
    //   overflow --> Msg_Err is returned
    // Upon return of Msg_None, bytes may be in the buffer0
    // Esp_ConsumeMessage removes the message from the buffer

    bool termStringFound = false;    
    unsigned long t_start = millis();
    bool msg = false;
    int len = strlen(termStr);
    while (!msg && (millis() < (t_start+tmax)) && (esp_ptr < esp_ptr_mx)) {
      if (esp.available()) {
        esp_buf[esp_ptr++] = esp.read();           
        if ((len > 0) && (esp_ptr >= len)) {
          msg = strncmp(termStr,&esp_buf[esp_ptr-len],len) == 0;
        }                
      }
    }
    for (int i = 0; i < esp_ptr; i++) {
      if (esp_buf[i] == '\r')  
        esp_buf[i] = '<';
      if (esp_buf[i] == '\n')
        esp_buf[i] = 'V';
    }
    esp_buf[esp_ptr] = '\0';
    if (msg)
      return Msg_Ok;
    else
      if (esp_ptr == esp_ptr_mx)
        return Msg_Err;
      else
        return Msg_None;
}

void Esp_Flush() {
  while (esp.available())
    esp.read();
}

void Esp_StartTcpServer() {
  bool err = false;
  short res;
  // hardware reset
  delay(2000);
  digitalWrite(rstPin,LOW);
  delay(2000);
  digitalWrite(rstPin,HIGH);
  delay(10000);
  Esp_Flush();  
  Esp_SendMessage("AT+RST"); 
  delay(10000);
  Esp_Flush();    
  Esp_SendMessage("AT+CWMODE=1"); 
  res = Esp_ReadMessage("OK\r\n",1000); Esp_ConsumeMessage(res);      
  err = err || (res != Err_Ok);
  Esp_SendMessage("AT+CWJAP=\"Schipper2\",\"7197358209076618\""); 
  res = Esp_ReadMessage("OK\r\n",10000); Esp_ConsumeMessage(res);
  err = err || (res != Err_Ok);
  Esp_SendMessage("AT+CIPMUX=1"); 
  res = Esp_ReadMessage("OK\r\n",1000); Esp_ConsumeMessage(res);
  err = err || (res != Err_Ok);
  Esp_SendMessage("AT+CIPSERVER=1,22"); 
  res = Esp_ReadMessage("OK\r\n",1000); Esp_ConsumeMessage(res);
  err = err || (res != Err_Ok);
  Esp_SendMessage("AT+CIPSTO=30"); 
  res = Esp_ReadMessage("OK\r\n",5000); Esp_ConsumeMessage(res);
  err = err || (res != Err_Ok);
  if (err)
    Serial.println(F("Error starting Tcp server"));  
  else
    Serial.println(F("Tcp server started..."));  
}

/*
void Esp_TcpServer() {
  // simple setup without any response processing
  
}


bool Esp_TermStringFound() {
  return termStringFound;
}

bool Esp_MsgFound() {
  return msgFound;  
}

bool Esp_ParseMessage(int *chn, int *cmd) {
  // parse a message of the form +IPD,<chn>,<len>,cmd
  // *chn is TCP channel
  // *cmd is type of cmd: 0 = unk, 1 = get, 2 = rst
  bool msg;
  msg = strncmp("+IPD,",esp_buf,5) == 0;
  *chn = esp_buf[5]-'0';
  *cmd = 0;
  if (strncmp("get",&esp_buf[9],3) == 0)
    *cmd = 1;
  else 
    if (strncmp("rst",&esp_buf[9],3) == 0)
      *cmd = 2;
  return msg;      
}


void Esp_SendPrep(int len, int chn) {    
  char buf[16];
  sprintf(buf,"AT+CIPSEND=%d,%d",chn,len+2);  
  Esp_Send(buf);
}

void Esp_PassResponse() {
  while (esp.available()) {
    Serial.write(esp.read());    
  }
}

void Esp_Flush(int tmax) {
  unsigned long t = millis();
  while (millis() < (t + tmax)) {
    if (esp.available())
      esp.read();
  }
}

void Esp_PrintMessage(bool consume) {
  Serial.print("> ");
  int len = strlen(esp_buf);
  for (int i = 0; i < len; i++) {
    if (esp_buf[i] == '\r')
      esp_buf[i] = '<';
    else
      if (esp_buf[i] == '\n')
        esp_buf[i] = 'V';
  }
  Serial.println(esp_buf);
  if (consume)
    Esp_ConsumeMessage();    
}



*/
