/*
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK -  pin 13
 ** CS -   pin 4 
 ** CS -   pin 10  ethernet
*/


/*---------------- declare ----------------*/
#include "Adafruit_Sensor.h"
#include "DHT.h"  
#include <SPI.h>
#include <Ethernet.h>    
#include <SD.h>

/* Pins */
#define SENSOR_DHT_PIN 2       // what digital pin we're connected to temparature sensor dht22
#define SENSOR_GAS_PIN 0       //int smoke_gas = 0;
#define CANAL1_PIN 8           //LED_RED_PIN
#define CANAL2_PIN 3
#define CANAL3_PIN 5
#define CANAL4_PIN 6
#define CANAL5_PIN 9          //pwd

#define REQ_BUF_SZ 40   

//объявляем датчик температуры dht22
DHT dht(SENSOR_DHT_PIN, DHT22);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177); 
EthernetServer server(80);           
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0};     // buffered HTTP request stored as null terminated string
char req_index = 0;                  // index into HTTP_req buffer

bool CANAL1;
bool CANAL2;
bool CANAL3;
bool CANAL4;
int CANAL5;

/*---------------- initialization ----------------*/
void setup() {
  pinMode (SENSOR_DHT_PIN, INPUT);
  pinMode (SENSOR_GAS_PIN, INPUT);
  
  Ethernet.begin (mac, ip);           //запускаем сервер с указанными ранее MAC и IP  
  server. begin();
  
  pinMode(CANAL1_PIN, OUTPUT);
  pinMode(CANAL2_PIN, OUTPUT);
  pinMode(CANAL3_PIN, OUTPUT);
  pinMode(CANAL4_PIN, OUTPUT);
  pinMode(CANAL5_PIN, OUTPUT);
  
  SD.begin(4);                      //cd-card
 
  dht.begin();                           //запускаем датчик температуры  
  
  CANAL1 = CANAL2 = CANAL3 = CANAL4 = 0; //сбрасываем каналы управления 
  
  Serial.begin (9600);                  //dbg
}

/*---------------- 3 часть ----------------*/
void loop() 
{
  
  // listen for incoming clients
  EthernetClient client = server.available();     
  if (client)      
  {
    boolean currentLineIsBlank = true;   // an http request ends with a blank line
    while (client.connected())  
    { 
      if (client.available())   
      { 
        char c = client.read(); 
        if (req_index < (REQ_BUF_SZ - 1)) 
        {
          HTTP_req[req_index] = c; // save HTTP request character
          req_index++;
        }
        
        if (c == '\n' && currentLineIsBlank)             
        //выводим HTML страницу
        {
          if (StrContains(HTTP_req, "GET /"))            
          {
            if (StrContains(HTTP_req, "/ ")              
                || StrContains(HTTP_req, "/index.htm"))  
            {
              sendHtmlFile(client, "index.htm");         
            } 
            else if (StrContains(HTTP_req, "/favicon.ico"))  
              {
                sendFile(client, "favicon.ico");
              } 
              else if (StrContains(HTTP_req, "/temp.png")) 
                {
                  sendFile(client, "temp.png");
                } 
                else if (StrContains(HTTP_req, "/humid.png")) 
                  {
                    sendFile(client, "humid.png");
                  } 
                  else if (StrContains(HTTP_req, "/flame.png")) 
                    {
                      sendFile(client, "flame.png");
                    } 
                    else if (StrContains(HTTP_req, "/style.css")) 
                      {
                        sendFile(client, "style.css");
                      } 
                      else if (StrContains(HTTP_req, "ajax_flame")) 
                        {
                          sendBaseAnswer(client);      
                                                    
                          
                          ////////////////////////////////////////////////////////////////////////////////////////////////
                          //отображение дачиков dht
                          float h = dht.readHumidity();      // Считывание влажности в процентах
                          float t = dht.readTemperature();  // Считывание температуры в градусах цельсия  
                          
                          float hic = dht.computeHeatIndex(t, h, false);// Расчет тепла по целсию
                          
                          int tInt = t; 
                          int hInt = h;
                          
                          displaySerial(t,h);

                          //////////////////////////////////////////////////////////////////////////////////////////////// 
                          
                          int sensorGAS = analogRead(SENSOR_GAS_PIN);                           

                          char buff[32];
                          sprintf(buff, "%d:%d:%d:%d:%d:%d:%d:%d:",
                                  sensorGAS, 
                                  tInt, 
                                  hInt,
                                  digitalRead(CANAL1_PIN), 
                                  digitalRead(CANAL2_PIN), 
                                  digitalRead(CANAL3_PIN), 
                                  digitalRead(CANAL4_PIN),
                                  CANAL5);
                          client.println(buff);                          
                          
                        } 
                        else if (StrContains(HTTP_req, "setpin?"))  
                          { 
                            if (StrContains(HTTP_req, "canal=1"))     
                            {
                              CANAL1 = !CANAL1;
                              digitalWrite(CANAL1_PIN, CANAL1);
                            } 
                            else if (StrContains(HTTP_req, "canal=2")) 
                              {
                                CANAL2 = !CANAL2;
                                digitalWrite(CANAL2_PIN, CANAL2);
                              } 
                              else if (StrContains(HTTP_req, "canal=3")) 
                                {
                                  CANAL3 = !CANAL3;
                                  digitalWrite(CANAL3_PIN, CANAL3);
                                } 
                                else if (StrContains(HTTP_req, "canal=4")) 
                                  {
                                    CANAL4 = !CANAL4;
                                    digitalWrite(CANAL4_PIN, CANAL4);
                                  } else if (StrContains(HTTP_req, "canal=5")) 
                                    {
                                      String input = HTTP_req;
                                      int posStart = input.indexOf("value=");
                                      int posEnd = input.indexOf(' ', posStart);
                                      String param = input.substring(posStart + 6, posEnd + 1);
                                      CANAL5 = param.toInt();
                                      analogWrite(CANAL5_PIN, CANAL5);
                                    }
                                sendBaseAnswer(client);
                          }  
          }
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);  
          break;
        }
        if (c == '\n') // you're starting a new line
        {          
          currentLineIsBlank = true;
        } 
        else if (c != '\r') 
          {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
      }
    }    
    // give the web browser time to receive the data
    delay(1);                      //время на получение новых данных
    // close the connection:
    client.stop();
  }
} 

/*---------------- functions ----------------*/
//отправляем html клиенту
bool sendHtmlFile(EthernetClient client, char *fileName) 
{
  webFile = SD.open(fileName);      
  sendBaseAnswer(client);           
  return loadFile(client, webFile);
}

//отправляем файл-ресурс клиенту
bool sendFile(EthernetClient client, char *fileName) 
{
  webFile = SD.open(fileName);
  client.println(F("HTTP/1.1 200 OK"));
  client.println();
  return loadFile(client, webFile);
}

//загружаем файл клиенту
bool loadFile(EthernetClient client, File &webFile) 
{
  if (webFile) 
  {
    while (webFile.available())
      client.write(webFile.read()); // send web page to client
    webFile.close();
    return 1;
  }
  return 0;
}

//отпрвляем базовый/заголовок ответ клиенту
void sendBaseAnswer(EthernetClient client) 
{
  //sendHttpOkAnswer(client);  
  client.println(F("HTTP/1.1 200 OK"));      //отправляем статус OK
  client.println(F("Content-Type: text/html")); 
  client.println(F("Connection: close"));
  client.println();  
}

//заполняеи строку нулями
void StrClear(char *str, char length) 
{
  for (int i = 0; i < length; i++) str[i] = 0;
}

//содержится ли в str строка sfind
char StrContains(char *str, char *sfind) 
{
  char found = 0;
  char index = 0;
  char len;
  len = strlen(str);
  if (strlen(sfind) > len) 
    return 0;
  while (index < len) 
  {
    if (str[index] == sfind[found]) 
    {
      found++;
      if (strlen(sfind) == found) 
        return 1;      
    }
    else found = 0;
    index++;
  }
  return 0;
}

//print info to console
void displaySerial(float t, float h)
{
  if (isnan(h) || isnan(t))                      
    {
      Serial.println("Read error DHT22");    
      return;                                  
    }
  Serial.print("Humidity: ");                
  Serial.print(h);                              
  Serial.print(" %\t");                         
  Serial.print("Temperature: ");                
  Serial.print(t);                              
  Serial.println(" *C ");                       
}
