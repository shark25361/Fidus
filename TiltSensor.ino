#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define START_ADDRESS 0 //address to start writing to EEPROM at
#define Speaker1 4
#define Speaker2 5
#define Speaker3 6
#define SPEAKER_FREQUENCY 500 //frequency of speakers sound

//int minVal=265;
//int maxVal=402;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
const int MPU_ADDR = 0x69; //I2C address of MPU6050
unsigned long x = 0,y = 0; //default tilt values to compare against
int address = START_ADDRESS;
String wifiSSID;
String wifiPASSWD;

void(* resetFunc)(void) = 0;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  while(!Serial)//wait for communications to start. not necessary 
  {
    
  }
  Wire.beginTransmission(MPU_ADDR);
  if(!SPIFFS.begin()) //start the server
  {
    //Serial.println("Error occured while mounting SPIFFS");
    return;
  }
  if(EEPROM.read(address) != 255) //if wifi creds are saved to internal EEPROM already
  {
    int SSID_LEN = EEPROM.read(address++);
    int PASSWD_LEN = EEPROM.read(address++);
    for(int i = 0;i < SSID_LEN;i++)
    {
      wifiSSID += EEPROM.read(address++);
    }
    wifiSSID += '\0';
    for(int i = 0;i < PASSWD_LEN;i++)
    {
      wifiPASSWD += EEPROM.read(address++);
    }
    wifiPASSWD += '\0';
    WiFi.begin(wifiSSID,wifiPASSWD);
    setupServer();
  }
  else
  {
    //Serial.println("Wifi credentials not found.");
  }
  //setup speakers to play sound
  if(Speaker1)
  {
    pinMode(Speaker1,OUTPUT);
  }
  if(Speaker2)
  {
    pinMode(Speaker2,OUTPUT);
  }
  if(Speaker3)
  {
    pinMode(Speaker3,OUTPUT);
  }
}

void loop() 
{
  Wire.requestFrom(MPU_ADDR,14,true);
  int16_t sumX = 0, sumY = 0, sumZ = 0;
  for(int i = 0;i < 10;i++)
  {
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    sumX += AcX;
    sumY += AcY;
    sumZ += AcZ;
    delay(100); //wait 100 milliseconds to prevent multiple inconsistencies
  }
  //int xAng = map(AcX,minVal,maxVal,-90,90);
  //int yAng = map(AcY,minVal,maxVal,-90,90);
   
  //x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  //y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);

  x= RAD_TO_DEG * (atan2(-AcY, -AcZ)+PI); //find roll or deviation from x axis
  y= RAD_TO_DEG * (atan2(-AcX, -AcZ)+PI); //find pitch or deviation from y axis

  Serial.println(x + " " + y);

  if(-3 <= x <= 3 || -3 <= y <= 3) //if tilt has reached unsafe levels
  {
    alertUser();
  }

  if(Serial.available() >= 1)
  {
    char input = Serial.read();
    actions(input);
  }
  delay(100); //wait 100 milliseconds to prevent web server from falling out of sync
}

void alertUser()
{
  if(Speaker1)
  {
    tone(Speaker1,SPEAKER_FREQUENCY);
    delay(10000); //play sound for 10 seconds
    noTone(Speaker1);
  }
  if(Speaker2)
  {
    tone(Speaker2,SPEAKER_FREQUENCY);
    delay(10000); //play sound for 10 seconds
    noTone(Speaker2);
  }
  if(Speaker3)
  {
    tone(Speaker2,SPEAKER_FREQUENCY);
    delay(10000); //play sound for 10 seconds
    noTone(Speaker3);
  }
}

void actions(char input) //interact with the arduino using the arduino.exe 
{
  switch(input)
  {
    case 'r':
      resetFunc();
      break;
    case 't':
      Serial.println(x);
      Serial.println(y);
      break;
     case 'w':
      char temp = Serial.read();
      while(temp != ' ')
      {
        wifiSSID += temp;
        char temp = Serial.read();
      }
      wifiSSID.concat('\0');
      while(temp != ' ')
      {
        wifiPASSWD += temp;
        char temp = Serial.read();
      }
      wifiPASSWD.concat('\0');
      EEPROM.write(address++,wifiSSID.length());
      EEPROM.write(address++,wifiPASSWD.length());
      for(int i = 0;i < wifiSSID.length();i++)
      {
        EEPROM.write(address++,wifiSSID[i]);
      }
      for(int i = 0;i < wifiPASSWD.length();i++)
      {
        EEPROM.write(address++,wifiPASSWD[i]);
      }
      address = 0;
      setupServer();
      break;
  }
}

void setupServer()
{
  AsyncWebServer server(80);
  WiFi.begin(wifiSSID,wifiPASSWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000); //waiting for connection
  }
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/roll", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(x).c_str());
  });
  server.on("/pitch", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/plain", String(y).c_str());
  });

  server.begin();
}
