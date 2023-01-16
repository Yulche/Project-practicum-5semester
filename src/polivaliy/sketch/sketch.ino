/*
   Sensors:
  dht22  - pin 2
  light  - pin A2
  soil   - pin  A0

  //пины для самодельного датчика наличия воды
  WATER_OUT_PIN pin 4
  WATER_IN_PIN pin 3

  pump - pin 7
  led green - pin 8
  led blue  - pin 6
  led red   - pin 5
*/



/*---------------- declare ----------------*/
#include "Adafruit_Sensor.h"
#include "DHT.h"

/* Pins */
#define PUMP_PIN 7
#define LAMP_PIN 8

//пины для самодельного датчика наличия воды
//#define WATER_OUT_PIN 4
#define SENSOR_WATER_IN_PIN 3

#define SENSOR_SOIL_PIN A0       //влажность почвы
#define SENSOR_DHT_PIN 2       // what digital pin we're connected to temparature sensor dht22
#define SENSOR_LIGHT_PIN A2

#define ON_WATER  10//принудительное включение подачи воды
#define ON_LIGHT  11//света

#define LED_BLUE_PIN 6

//настройка длительности полива, мс
#define WATERING_MILLIS 100

//настройка пороговых значений для климатических показателей
#define SOIL_THRESHOLD 100
#define TEMP_THRESHOLD 100
#define LIGHT_THRESHOLD 550

//переменные для хранения климатических показателей
int soil_humidity;
int lightness;
boolean gotWater;

//объявляем датчик температуры dht22
DHT dht(SENSOR_DHT_PIN, DHT22);
float dhtTemp, dhtHum;
int totime = 0;      //сколько прошло времени в млс
int timeLamp = 0;
int timing = 1;

/*---------------- initialization ----------------*/
void setup() {

  //pinMode(WATER_OUT_PIN, OUTPUT);
  //входящий пин от датчика воды будет подтянут к питанию
  //т.е. срабатывание будет, если есть контакт при исходящем пине в состоянии LOW
  //pinMode(WATER_IN_PIN, INPUT_PULLUP);

  pinMode(SENSOR_WATER_IN_PIN, INPUT);
  gotWater = false;

  pinMode(SENSOR_DHT_PIN, INPUT);
  pinMode(SENSOR_LIGHT_PIN, INPUT);
  pinMode(SENSOR_SOIL_PIN, INPUT);

  pinMode(ON_WATER, INPUT);
  pinMode(ON_LIGHT, INPUT);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  //dht.begin();                           //запускаем датчик температуры
  Serial.begin (9600);                  //dbg

}


/*---------------- execution ----------------*/
void loop() {
  //считываем данные с dht
  //delay(2000);                                 // Задежка в 2 с

  //считывае данные с датчиков
  dhtTemp = dht.readTemperature();               // Считывание температуры в градусах цельсия
  dhtHum = dht.readHumidity();                   // Считывание влажности в процентах


  //float hic = dht.computeHeatIndex(t, h, false); // Расчет тепла по целсию

  int tInt = dhtTemp;  //в целых значениях
  int hInt = dhtHum;

  soil_humidity = analogRead(SENSOR_SOIL_PIN);
  lightness = analogRead(SENSOR_LIGHT_PIN);
  gotWater = checkWater();

  //Включаем работу лампы с задержкой
  if(millis()/1000 - timeLamp >  timing*2){ 
    timeLamp = lamping();
  } 

  //если земля сухая, то поливаем
  if (soil_humidity < SOIL_THRESHOLD) {
    watering();
  }

  //нет воды
  if (!gotWater) {
    digitalWrite(LED_BLUE_PIN, HIGH);
  }
  else {
    digitalWrite(LED_BLUE_PIN, LOW);
  }

  //принудительное включение
  if(digitalRead(ON_WATER)){
    watering();
  }

  
  displaySerial();

}





/*---------------- functions ----------------*/
boolean checkWater()
{
  /*  digitalWrite(WATER_OUT_PIN, LOW);
    //если после переключения исходящего пина датчика воды в LOW
    //контакта со входящим пином нет, на входящем будет HIGH
    if (digitalRead(WATER_IN_PIN))
      //значит, воды нет
      gotWater = false;
    else
      //в противном случае - есть
      gotWater = true;
    digitalWrite(WATER_OUT_PIN, HIGH);
    //несмотря на глобальность gotWater,
    //вернем ее значение, чтобы в состоянии "нет воды"
    //проверять наличие воды и сразу на него реагировать
    return gotWater;
  */
  return digitalRead(SENSOR_WATER_IN_PIN);
}

//полив: включение помпы и ее выключение
//после заданного в настройках времени, если есть вода
void watering() {

  if (checkWater() ) {
    Serial.println("++++++++++++pomp+++++++++++");
    digitalWrite(PUMP_PIN, HIGH);
    delay(WATERING_MILLIS);
    digitalWrite(PUMP_PIN, LOW);
    delay(WATERING_MILLIS*2);
  }
}

//управление лампой
int lamping() {  
  //если не хватает освещения, то включаем лампу
  if( (lightness + 50 < LIGHT_THRESHOLD)||(digitalRead(ON_LIGHT) ) ){
    digitalWrite(LAMP_PIN,  HIGH);
  } else if (lightness > LIGHT_THRESHOLD) {
    digitalWrite(LAMP_PIN,  LOW);
  }
  return millis() / 1000;
}


//*
//вывод отладочной информации в последовательный порт
void show_watering() {
  //вывод отладочной информации в последовательный порт
  //displaySerial(dhtTemp,dhtHum);
  Serial.print("soil_humidity = ");
  Serial.println (soil_humidity);
  Serial.print("checkWater = ");
  Serial.println( checkWater() );
  Serial.print("gotWater = ");
  Serial.println(gotWater);
}

void show_light() {
  Serial.print("light = ");
  Serial.println(lightness);
}

void show_temp(float t, float h) {
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

void displaySerial() {
  if ( millis() / 1000 - totime > timing  ) { // задержка вывода
    totime = millis() / 1000;

    show_watering();
    show_light();
  }
}
