#include <SPI.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


#define CSB_DDS 15
#define UPD_DDS 5
#define RST_DDS 4
 #define SYSFREQ_DDS 800137209

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title></title>
</head>
<body>
<h2>AD9910 SIGNAL GENERATOR</h2>

<form action="/action_page">
<p>Frequency:<br />
<input name="freq" type="text" value="1000" />&nbsp; Hz<br />
Amplitude:<br />
<input name="amp" type="text" value="1000" />&nbsp; mV<br />
<br />
<input type="submit" value="Submit" /></p>
</form>
<form action="/reset">
<p><input name="init" type="submit" value="Reset DDS" /></p>
</form>
</body>
</html>
)=====";

const char* ssid = "YOUR_WIFI_SSID";    //  Your Wi-Fi Name

const char* password = "WIFI_PASSWORD";   // Wi-Fi Password

 

ESP8266WebServer server(80); //Server on port 80

 SPISettings settingsA(1000, MSBFIRST, SPI_MODE0);

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleReset() {
  initDDS();
   String s = "<a href='/'> Reset done </a>";
 server.send(200, "text/html", s); //Send web page
}


void handleForm() {
 String freq = server.arg("freq"); 
 String amp = server.arg("amp"); 

 Serial.print("Frequency:");
 Serial.println(freq);
 int str_len = freq.length()+1; 
 char char_array[str_len];
 freq.toCharArray(char_array, str_len);
 setDDSfreq(atol(char_array));
 
 Serial.print("Amplitude:");
 Serial.println(amp);
  str_len = amp.length()+1; 
 char char_array1[str_len];
 amp.toCharArray(char_array1, str_len);
 setDDSamp(atol(char_array1));
 
 String s = "<a href='/'> Go Back </a>";
 server.send(200, "text/html", s); //Send web page
}


void setup(void){
  Serial.begin(115200);
   pinMode(UPD_DDS, OUTPUT);
  digitalWrite(UPD_DDS, LOW);

     pinMode(CSB_DDS, OUTPUT);
  digitalWrite(CSB_DDS, HIGH);
  
     pinMode(RST_DDS, OUTPUT);
  digitalWrite(RST_DDS, LOW);
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      
  server.on("/action_page", handleForm); 
  server.on("/reset", handleReset); 

 
  server.begin();                  //Start server
  Serial.println("HTTP server started");

  SPI.begin();
      delay(100);

  initDDS();
    delay(100);

}

void initDDS(){
    digitalWrite(RST_DDS, HIGH);
    delay(100);
      digitalWrite(RST_DDS, LOW);

  char data[8];
  data[0] = 0x00;
  data[1] = 0x00;
  data[2] = 0x42;
  data[3] = 0x02;
  writeDDSreg(0x00, 4,data);

  data[0] = 0x35;
  data[1] = 0x1f;
  data[2] = 0x41;
  data[3] = 0x28;
  writeDDSreg(0x02, 4,data);
  delay(100);
  
  data[0] = 0x01;
  data[1] = 0x00;
  data[2] = 0x08;
  data[3] = 0x20;
  writeDDSreg(0x01, 4,data);

  data[0] = 0x00;
  data[1] = 0x00;
  data[2] = 0x00;
  data[3] = 0x7f;
  writeDDSreg(0x03, 4,data);
}

void updateDDS(){
  digitalWrite(UPD_DDS, HIGH);
  delay(10);
  digitalWrite(UPD_DDS, LOW);
  delay(10);
}

//in data MSB = data[0]
void writeDDSreg(char addr, char bytes,char * data){
  digitalWrite(CSB_DDS, LOW);
  SPI.beginTransaction(settingsA);
  SPI.transfer(addr);
  for(int i=0;i<bytes;i++){
    SPI.transfer(data[i]);
  }
  SPI.endTransaction();
  updateDDS();  
  digitalWrite(CSB_DDS, HIGH);

}


void setDDSfreq(unsigned long freq){

  unsigned long FTW = (pow(2,32)*freq)/SYSFREQ_DDS;
  char data[8];


  data[0] = 0x00;
  data[1] = 0x00;
  data[2] = 0x00;
  data[3] = 0x00;
  
  data[4] = ((FTW>>24)&0xFF);
  data[5] = ((FTW>>16)&0xFF);
  data[6] = ((FTW>>8)&0xFF);
  data[7] = (FTW&0xFF);
    
  writeDDSreg(0x0E, 8,data);

}

void setDDSamp(unsigned long amp){
  
  unsigned long AMP = (amp*65535)/2680;
  
  if(AMP>65535){
   AMP =65535;  
  }

  
  
  char data[4];

  data[0] = ((AMP>>24)&0xFF);
  data[1] = ((AMP>>16)&0xFF);
  data[2] = ((AMP>>8)&0xFF);
  data[3] = (AMP&0xFF)|0x3;
  writeDDSreg(0x09, 4,data);

}

void loop(void){
  server.handleClient();          //Handle client requests
  
}
