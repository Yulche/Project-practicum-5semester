/*
SD card 
Ethernet Shield
SPI bus:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK -  pin 13
 ** CS -   pin 4   SDCARD _SS_PIN
 ** CS -   pin 10  ethernet
 
BMP 280 
I2C bus:
* sda - pin A4
* scl - pin A5
* gnd
* vcc - 3.3v

led green - pin 8
led white - pin 7
led blue  - pin 6
led red   - pin 5

dht22   - pin 2 
 
*/


/*---------------- declare ----------------*/
#include "Adafruit_Sensor.h"
#include "DHT.h" 

#include <Wire.h> 
#include <SPI.h>

#include "Adafruit_BMP280.h"

#include <Ethernet.h>    //библиотека для работы с Ethernet
#include <SD.h>

/* Pins */
#define SENSOR_DHT_PIN 2       // what digital pin we're connected to temparature sensor dht22
#define SENSOR_GAS_PIN 0       //int smoke_gas = 0;               //пин на котором подключен MQ-7
#define CANAL1_PIN 8           //LED_GREEN_PIN
#define CANAL2_PIN 3           //white
#define CANAL3_PIN 6           //blue
#define CANAL4_PIN 5           //red
#define CANAL5_PIN 9           //pwd

#define REQ_BUF_SZ 40   //размер буфера

//объявляем датчик температуры dht22
DHT dht(SENSOR_DHT_PIN, DHT22);

//объявляем датчик bmp 280
// Для подключения по шине I2C
//Adafruit_BMP280 bmp;
// Для подключения по аппаратному SPI (указываем только номер пина CS)
/*
#define PIN_CS 10
Adafruit_BMP280 bmp(PIN_CS);
*/
// Для подключения по программному SPI (указываем все пины интерфейса)
/*
#define PIN_SCK  13
#define PIN_MISO 12
#define PIN_MOSI 11
#define PIN_CS   10
*/
//Adafruit_BMP280 bmp(PIN_CS, PIN_MOSI, PIN_MISO,  PIN_SCK);


//вводим mac адрес
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//вводим IP адрес сервера
IPAddress ip(192, 168, 1, 177); 
EthernetServer server(80);           //инифиализация библиотеки Ethernet server library
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0};     // buffered HTTP request stored as null terminated string
char req_index = 0;                  // index into HTTP_req buffer

bool CANAL1;
bool CANAL2;
bool CANAL3;
bool CANAL4;
char CANAL5;               //to 255

int  bmpTemp, bmpPress, dhtTemp, dhtHum; //
/*---------------- initialization ----------------*/
void setup() 
{
  pinMode (SENSOR_DHT_PIN, INPUT);
  pinMode (SENSOR_GAS_PIN, INPUT);
  
  Ethernet.begin (mac, ip);//запускаем сервер с указанными ранее MAC и IP  
  server. begin();
  
  pinMode(CANAL1_PIN, OUTPUT);
  pinMode(CANAL2_PIN, OUTPUT);
  pinMode(CANAL3_PIN, OUTPUT);
  pinMode(CANAL4_PIN, OUTPUT);
  pinMode(CANAL5_PIN, OUTPUT);
  
  SD.begin(4);                      //cd-card
  Serial.begin (9600);                  //dbg

  dht.begin();                           //запускаем датчик температуры  
  
  CANAL1 = CANAL2 = CANAL3 = CANAL4 = 0; //сбрасываем каналы управления 
/*  
  if(!bmp.begin()) // Если датчик BMP280 не найден
  { 
    //Serial.println("BMP280 SENSOR ERROR"); // Выводим сообщение об ошибке
    while(1); // Переходим в бесконечный цикл
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,   // Режим работы
                Adafruit_BMP280::SAMPLING_X2,     // Точность изм. температуры
                Adafruit_BMP280::SAMPLING_X16,    // Точность изм. давления
                Adafruit_BMP280::FILTER_X16,      // Уровень фильтрации
                Adafruit_BMP280::STANDBY_MS_500); // Период просыпания, мСек
*/                
  /*
  Первый параметр отвечает за режим работы датчика. Всего доступно 4 варианта, а именно:

    MODE_NORMAL – в данном режиме модуль циклически выходит из режима сна через установленный интервал времени. В активном состоянии он проводит измерения, сохраняет их в своей памяти и заново уходит в сон.
    MODE_FORCED – в этом режиме датчик проводит измерения при получении команды от Arduino, после чего возвращается в состояние сна.
    MODE_SLEEP – режим сна или пониженного энергопотребления.
    MODE_SOFT_RESET_CODE – сброс на заводские настройки.

Второй и третий параметры отвечают за точность измерения температуры и атмосферного давления соответственно. Они могут принимать следующие значения:

    SAMPLING_NONE - минимальная точность;
    SAMPLING_X1 – точность АЦП 16 бит;
    SAMPLING_X2 – точность АЦП 17 бит;
    SAMPLING_X4 – точность АЦП 18 бит;
    SAMPLING_X8 – точность АЦП 19 бит;
    SAMPLING_X16 – точность АЦП 20 бит.

Четвёртый параметр отвечает за уровень фильтрации измеренных данных. Значения этого параметра могут быть следующие:

    FILTER_OFF – фильтр выключен;
    FILTER_X2 – минимальный уровень фильтрации;
    FILTER_X4;
    FILTER_X8;
    FILTER_X16 – максимальный уровень фильтрации.

Последний, пятый параметр функции setSampling(...) отвечает за период перехода модуля в активное состояние с целью выполнения измерений. Параметр может принимать следующие значения:

    STANDBY_MS_1 – модуль просыпается каждую миллисекунду;
    STANDBY_MS_63 – модуль просыпается каждые 63 миллисекунды;
    STANDBY_MS_125 – модуль просыпается каждых 125 миллисекунд;
    STANDBY_MS_250 – модуль просыпается каждых 250 миллисекунд;
    STANDBY_MS_500 – модуль просыпается каждых 500 миллисекунд;
    STANDBY_MS_1000 – модуль просыпается каждую секунду;
    STANDBY_MS_2000 – модуль просыпается каждые 2 секунды;
    STANDBY_MS_4000 – модуль просыпается каждых 4 секунды;
  */

}

/*---------------- 3 часть ----------------*/
void loop() 
{
  
  //считываем данные с BMP280
/*  bmpTemp = bmp.readTemperature(); // Функция измерения температуры
  bmpPress = bmp.readPressure() * 0.00750062;  // Функция измерения атм. давления
*/  
  //считываем данные с dht
  //delay(2000);                                 // Задежка в 2 с
  
  dhtTemp = dht.readTemperature();               // Считывание температуры в градусах цельсия  
  dhtHum = dht.readHumidity();                   // Считывание влажности в процентах
  Serial.println(dhtTemp);

  /*
  float h = dht.readHumidity();                  
  float t = dht.readTemperature();               // Считывание температуры в градусах цельсия  
  
  //float hic = dht.computeHeatIndex(t, h, false); // Расчет тепла по целсию
  
  int tInt = t; 
  int hInt = h;
  */  
  //вывод отладочной информации в последовательный порт
  //displaySerial(bmpTemp, bmpPress,t,h);
  
  // listen for incoming clients
  EthernetClient client = server.available();     //принимаем данные, посылаемые клиентом
  if (client)      //если запрос оканчивается пустой строкой
  {
    boolean currentLineIsBlank = true;   // an http request ends with a blank line
    //ставим метку об окончании запроса (дословно: текущая линия чиста)
    while (client.connected())  //пока есть соединение с клиентом
    { 
      if (client.available())   //если клиент активен
      { 
        char c = client.read(); //считываем посылаемую информацию в переменную "с"
        //и по символьно добовляем в буфер
        if (req_index < (REQ_BUF_SZ - 1)) 
        {
          HTTP_req[req_index] = c; // save HTTP request character
          req_index++;
        }
        
        if (c == '\n' && currentLineIsBlank)             //выводим HTML страницу
        {
          if (StrContains(HTTP_req, "GET /"))            //смотрим есть в запросе метод "GET /"
          {
            if (StrContains(HTTP_req, "/ ")               //смотрим есть в запросе "/ "
                || StrContains(HTTP_req, "/index.htm"))   //смотрим есть в запросе "/index.htm"
            {
              sendHtmlFile(client, "index.htm");           //выводим HTML страницу
            } 
            else if (StrContains(HTTP_req, "/favicon.ico"))  //подгружаем ресурсы
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
                          /*
                          //считываем данные с BMP280
                          bmpTemp = bmp.readTemperature(); // Функция измерения температуры
                          bmpPress = bmp.readPressure() * 0.00750062;  // Функция измерения атм. давления
                          
                          //считываем данные с dht
                          //delay(2000);                                 // Задежка в 2 с
                          
                          dhtTemp = dht.readTemperature();               // Считывание температуры в градусах цельсия  
                          dhtHum = dht.readHumidity();                   // Считывание влажности в процентах
                          */
                          //////////////////////////////////////////////////////////////////////////////////////////////// 
                          
                          int sensorGAS = analogRead(SENSOR_GAS_PIN);                           

                          char buff[7*4];
                          sprintf(buff, "%d:%d:%d:%d:%d:%d:%d:%d:",
                                  //sensorGAS,
                                  //bmpPress, 
                                  dhtTemp, 
                                  dhtHum,
                                  digitalRead(CANAL1_PIN), 
                                  digitalRead(CANAL2_PIN), 
                                  digitalRead(CANAL3_PIN), 
                                  digitalRead(CANAL4_PIN),
                                  CANAL5);
                          client.println(buff);                          
                          
                        } 
                        else if (StrContains(HTTP_req, "setpin?"))  //проверяем есть ли в запросе "setpin?"
                          { //отрабатываем нажатие кнопок
                            if (StrContains(HTTP_req, "canal=1"))     //обрабатываем первый канал
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
          StrClear(HTTP_req, REQ_BUF_SZ);  //очищаем буфер
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
    client.stop();                 //закрываем соеднение
  }
} 
  
  


/*---------------- functions ----------------*/
//отправляем html клиенту
bool sendHtmlFile(EthernetClient client, char *fileName) 
{
  webFile = SD.open(fileName);      //открываем файл
  sendBaseAnswer(client);           //отпрвляем базовый ответ клиенту
  return loadFile(client, webFile);
}

//отправляем файл-ресурс клиенту
bool sendFile(EthernetClient client, char *fileName) 
{
  webFile = SD.open(fileName);
  //sendHttpOkAnswer(client);
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
{  //header    
  client.println(F("HTTP/1.1 200 OK"));      //отправляем статус OK
  client.println(F("Content-Type: text/html")); //F - специальным образом размещаем в памяти
  client.println(F("Connection: close"));
  client.println();  //
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

/*
//вывод отладочной информации в последовательный порт
void displaySerial(int bmpTemp, int bmpPress, float t, float h)
{
  //BMP280 sensor
  // Выводим значение температуры
  Serial.print(F("Temperature = "));
  Serial.print(bmpTemp); // Функция измерения температуры
  Serial.println(" *C");

  // Выводим значение атмосферного давления
  Serial.print(F("Pressure = "));
  Serial.print(bmpPress);  // Функция измерения атм. давления
  Serial.println(" Pa");
/*
  // Выводим значение высоты
  Serial.print(F("Approx altitude = "));
  Serial.print(bmp.readAltitude(1013.25)); // Функция измерения высоты
  Serial.println(" m");  
*/  
/*
 //DHT22 sensor
  if (isnan(h) || isnan(t))                     // Проверяем, получилось считать данные 
    {
      Serial.println("Read error DHT22");    // Выводим текст
      return;                                  
    }
  Serial.println("DHT22 sensor");
  Serial.print("Humidity: ");                   // Выводим текст
  Serial.print(h);                              // Отображаем влажность
  Serial.print(" %\t");                         // Выводим текст
  Serial.print("Temperature: ");                // Выводим текст
  Serial.print(t);                              // Отображаем температуру
  Serial.println(" *C ");                       // Выводим текст
}
*/
