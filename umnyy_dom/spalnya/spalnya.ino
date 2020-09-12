#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include<FS.h>
#include<ArduinoJson.h>
#include "DHT.h"
#define PIN_BUTTON_SPALNYA D3
#define PIN_BUTTON_CABINET D4
#define PIN_SVET_SPALNYA_1 D2
#define PIN_SVET_SPALNYA_2 D8
#define PIN_SVET_CABINET_1 D9
#define PIN_SVET_CABINET_2 D10



#define DHTPIN D7     // Номер пина, который подключен к DHT22
#define DHTTYPE DHT22   // Указываем, какой тип датчика мы используем
bool lustra;
enum Status_Svet
{
  Off=0,
  One=1,
  Two=2,
  All=3
};
Status_Svet Spalnya_status;
Status_Svet Cabinet_status;
bool podogrev = LOW;
const char* ssid = "********";
const char* password = "********";
ESP8266WebServer server(80);
IPAddress ip(192, 168, 0, 17); //статический IP
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
DHT dht(DHTPIN, DHTTYPE);
HTTPClient http;

float t_zh = 22.0;
float delta = 0.1;
float delta_change = 0.1;
String jsonConfig = "{}";
float currentTemperature;
float currentHumidity;
long currentSensor;
unsigned long lastRequestTemperature;
String build_string_status(Status_Svet st)
{
  String res ="";
  if(st==Off)
    res = "Выключен";
    if(st==One)
    res = "Включен 1";
    if(st==Two)
    res = "Включен 2";
    if(st==All)
    res = "Включен Все";
    return res;
}
void handleRoot() {
  loadConfig();
  String s = "<h1>Свет в спальне ";
  s += build_string_status(Spalnya_status);
  s += "</h1>";
  s += "<h2><a href=\"/led1/on1\">Включить 1</a> ";
  s += "<a href=\"/led1/on2\">Включить 2</a></h2>";
  s += "<h2><a href=\"/led1/off1\">Выключить 1</a> ";
  s += "<a href=\"/led1/off2\">Выключить 2</a></h2>";
  s += "<h1>Свет в детской ";
   s += build_string_status(Cabinet_status);
  s += "</h1>";
  s += "<h2><a href=\"/led2/on1\">Включить 1</a> ";
  s += "<a href=\"/led2/on2\">Включить 2</a></h2>";
  s += "<h2><a href=\"/led2/off1\">Выключить 1</a> ";
  s += "<a href=\"/led2/off2\">Выключить 2</a></h2>";
  s += "<h1>Температура в спальне ";
  s += String(currentTemperature);
  s += "'C";
  s += "</h1>";
  s += "<h1>Влажность в спальне ";
  s += String(currentHumidity);
  s += "%";
  s += "</h1>";
  s += "<h1>Подогрев ";
  s += (podogrev) ? "включен" : "выключен";
  s += "</h1>";
  s += "<h1>Шаг изменения температуры ";
  s += (String(delta_change));
  s += "</h1>";
  s += "<h2><a href=\"/delta_change_increase\">Увеличить</a> ";
  s += "<a href=\"/delta_change_decrease\">Уменьшить</a></h2>";
  s += "<h1>Желаемая температура ";
  s += (String(t_zh));
  s += " 'C";
  s += "</h1>";
  s += "<h2><a href=\"/temperature_increase\">Увеличить</a> ";
  s += "<a href=\"/temperature_decrease\">Уменьшить</a></h2>";
  s += "<h1>Допустимое отклонение ";
  s += (String(delta));
  s += "</h1>";
  s += "<h2><a href=\"/delta_increase\">Увеличить</a> ";
  s += "<a href=\"/delta_decrease\">Уменьшить</a></h2>";
  s += "<h2><a href=\"/davlenie\">Прибавить давление</a> ";
  s += "</h2>";
  s += "<h1>Режим люстры ";
  s += (lustra) ? "включен" : "выключен";
  s += "</h1>";
  s += "<h2><a href=\"/rezhim_lustra\">Переключить режим люстры</a> ";
  s += "</h2>";
  server.send(200, "text/html; charset=utf-8", s);
}

void change_value(String type, int inc) {
  if (type == "t")
  {
    t_zh = t_zh + delta_change * inc;
  }
  if (type == "delta")
  {
    delta = delta + 0.1 * inc;
  }
  if (type == "delta_change")
  {
    delta_change = delta_change + 0.1 * inc;
  }
  if (type == "lustra")
  {
    lustra = inc;
  }
  saveConfig();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}


void delta_change_increase()
{
  change_value("delta_change", 1);
}
void delta_change_decrease()
{
  change_value("delta_change", -1);
}
void temperature_increase()
{
  change_value("t", 1);
}
void temperature_decrease()
{
  change_value("t", -1);
}
void delta_increase()
{
  change_value("delta", 1);
}
void delta_decrease()
{
  change_value("delta", -1);
}
void rezhim_lustra()
{
  change_value("lustra",!lustra);
}

bool loadConfig() {
  // Открываем файл для чтения
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    // если файл не найден
    Serial.println("Failed to open config file");
    //  Создаем файл запиав в него аные по умолчанию
    saveConfig();
    return false;
  }
  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }
  // загружаем файл конфигурации в глобальную переменную
  jsonConfig = configFile.readString();
  // Резервируем памяь для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266
  DynamicJsonBuffer jsonBuffer;
  //  вызовите парсер JSON через экземпляр jsonBuffer
  //  строку возьмем из глобальной переменной String jsonConfig
  JsonObject& root = jsonBuffer.parseObject(jsonConfig);
  // Теперь можно получить значения из root
  t_zh = root["t_zh"];
  delta = root["delta"];
  delta_change = root["delta_change"];
  lustra = root["lustra"];
  return true;
}

bool saveConfig() {
  // Резервируем памяь для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266
  DynamicJsonBuffer jsonBuffer;
  //  вызовите парсер JSON через экземпляр jsonBuffer
  JsonObject& json = jsonBuffer.parseObject(jsonConfig);
  // Заполняем поля json
  json["t_zh"] = t_zh;
  json["delta"] = delta;
  json["delta_change"] = delta_change;
  json["lustra"] = lustra;
  // Помещаем созданный json в глобальную переменную json.printTo(jsonConfig);
  json.printTo(jsonConfig);
  // Открываем файл для записи
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  // Записываем строку json в файл
  json.printTo(configFile);
  return true;
}
Status_Svet int_to_Status_Svet(int x)
{
  if(x==0)
  return Off;
  if(x==1)
  return One;
  if(x==2)
  return Two;
  if(x==3)
  return All;
}
Status_Svet switch_status(Status_Svet st)
{
  if(st==Off)
  {
    if(!lustra)
      return All;
     return One;
  }
    if(st==One)
    { 
      if(!lustra)
        return Off;
      return Two;
    }
    if(st==Two)
    {
      if(!lustra)
        return Off;
      return All;
    }
    if(st==All)
      return Off;
}
Status_Svet switch_status(Status_Svet st,bool on,int number)
{
  if(on)
  {
     if(st==Off||st==int_to_Status_Svet(number))
     {
       return int_to_Status_Svet(number);
     }
       else return All;
  } else
    {
      if(st==All||int_to_Status_Svet(All-number)==st)
      {
       return int_to_Status_Svet(All-number);
      }
       else return Off;
    }
}
int interval = 1000;
void send_command(String command)
{
  Serial.print(command);
  String inString="";
  unsigned long start = millis();
  while(abs(millis()-start)<3000)
  {
  while (Serial.available() > 0) {
    inString=Serial.readString();
    if(inString=="OK")
      return;
  }
  }
}
void sendInfoToUno(String room,Status_Svet st)
{
    if (st==Off)
    {
      if(room=="spalnya")
      {
        digitalWrite(PIN_SVET_SPALNYA_1,LOW);
        digitalWrite(PIN_SVET_SPALNYA_2,LOW);
      }
      else
      {
        digitalWrite(PIN_SVET_CABINET_1,LOW);
        digitalWrite(PIN_SVET_CABINET_2,LOW);
      }
    }
    if(st==One)
    {
      if(room=="spalnya")
      {
        digitalWrite(PIN_SVET_SPALNYA_1,HIGH);
        digitalWrite(PIN_SVET_SPALNYA_2,LOW);
      }
      else
      {
        digitalWrite(PIN_SVET_CABINET_1,HIGH);
        digitalWrite(PIN_SVET_CABINET_2,LOW);
      }
    }
    if(st==Two)
    {
      if(room=="spalnya")
      {
        digitalWrite(PIN_SVET_SPALNYA_1,LOW);
        digitalWrite(PIN_SVET_SPALNYA_2,HIGH);
      }
      else
      {
        digitalWrite(PIN_SVET_CABINET_1,LOW);
        digitalWrite(PIN_SVET_CABINET_2,HIGH);
      }
    }
    if(st==All)
    {
      if(room=="spalnya")
      {
        digitalWrite(PIN_SVET_SPALNYA_1,HIGH);
        digitalWrite(PIN_SVET_SPALNYA_2,HIGH);
      }
      else
      {
        digitalWrite(PIN_SVET_CABINET_1,HIGH);
        digitalWrite(PIN_SVET_CABINET_2,HIGH);
      }
    }
    
}

// Метод включения диода
void ledOn(int x,int number) {
  if (x == 1)
  {
    Spalnya_status = switch_status(Spalnya_status,true,number);
    sendInfoToUno("spalnya",Spalnya_status);      
  }
  if (x == 2)
  {
   Cabinet_status = switch_status(Cabinet_status,true,number);
   sendInfoToUno("cabinet",Cabinet_status);
  }
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}
void ledOff(int x,int number) {
  if (x == 1)
  {
    Spalnya_status = switch_status(Spalnya_status,false,number);
    sendInfoToUno("spalnya",Spalnya_status);
  }
  if (x == 2)
  {
    Cabinet_status = switch_status(Cabinet_status,false,number);
    sendInfoToUno("cabinet",Cabinet_status);
  }
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

// Метод выключения диода
void led1on1()
{
  ledOn(1,1);
}
void led1on2()
{
  ledOn(1,2);
}
void led2on1()
{
  ledOn(2,1);
}
void led2on2()
{
  ledOn(2,2);
}
void led1off1()
{
  ledOff(1,1);
}
void led1off2()
{
  ledOff(1,2);
}
void led2off1()
{
  ledOff(2,1);
}
void led2off2()
{
  ledOff(2,2);
}







void checkButtons()
{
  if(digitalRead(PIN_BUTTON_SPALNYA)==HIGH)
    {
      Spalnya_status = switch_status(Spalnya_status);
      sendInfoToUno("spalnya",Spalnya_status);
    }
    if(digitalRead(PIN_BUTTON_CABINET)==HIGH)
    {
      Cabinet_status = switch_status(Cabinet_status);
      sendInfoToUno("cabinet",Cabinet_status);
    }
}

void setup() {
  delay(1000);
  pinMode(PIN_BUTTON_SPALNYA, INPUT);
  pinMode(PIN_BUTTON_CABINET, INPUT);
  pinMode(BUILTIN_LED,OUTPUT);
  pinMode(PIN_SVET_SPALNYA_1,OUTPUT);
  pinMode(PIN_SVET_SPALNYA_2,OUTPUT);
  pinMode(PIN_SVET_CABINET_1,OUTPUT);
  pinMode(PIN_SVET_CABINET_2,OUTPUT);
  Spalnya_status=Off;
  Cabinet_status=Off;
  bool diod=true;
  delay(1000);
  dht.begin();
  Serial.begin(115200);
  SPIFFS.begin();
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial.println("");
  // ожидание соединения
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    diod= !diod;
    digitalWrite(BUILTIN_LED,diod);
    Serial.print(".");
  }
  digitalWrite(BUILTIN_LED,true);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/led1/on1", led1on1);
  server.on("/led1/on2", led1on2);
  server.on("/led1/off1", led1off1);
  server.on("/led1/off2", led1off2);
  server.on("/led2/on1", led2on1);
  server.on("/led2/on2", led2on2);
  server.on("/led2/off1", led2off1);
  server.on("/led2/off2", led2off2);
  server.on("/delta_change_increase", delta_change_increase);
  server.on("/delta_change_decrease", delta_change_decrease);
  server.on("/temperature_increase", temperature_increase);
  server.on("/temperature_decrease", temperature_decrease);
  server.on("/delta_increase", delta_increase);
  server.on("/delta_decrease", delta_decrease);
  server.on("/davlenie", davlenie);
  server.on("/rezhim_lustra",rezhim_lustra);
  server.begin();
  Serial.println("HTTP server started");
  currentTemperature = temperature();
  lastRequestTemperature = millis();
}
unsigned long lastCheckButtons=0;
void loop() {
  unsigned long currentTime = millis();
  if (abs(currentTime - lastCheckButtons) > 500)
  {
    checkButtons();
    lastCheckButtons=millis();
  }
  if (abs(currentTime - lastRequestTemperature) > 2000)
  {
    currentTemperature = temperature();
    currentHumidity = humidity();
    checkButtons();
    lastRequestTemperature = millis();
  }
  server.handleClient();
  if (podogrev
  )
  {
    if (currentTemperature > (t_zh + delta))
    {
      podogrevOff();
    }
  }
  else
  {
    if (currentTemperature < t_zh - delta)
    {
      podogrevOn();
    }
  }
}

float temperature()
{
    return dht.readTemperature(); // Температура
}

float humidity()
{
  return dht.readHumidity(); // Влажность
}


void davlenie()
{
  http.begin("http://192.168.0.18/davlenie");
  http.addHeader("Content-Type", "text/plain");
  if (http.GET() <= 0)
  {
    http.end();
    return;
  }
}

void podogrevOn()
{
  http.begin("http://192.168.0.18/kotel/on");
  http.addHeader("Content-Type", "text/plain");
  if (http.GET() <= 0)
  {
    http.end();
    return;
  }
  podogrev = HIGH;
}
void podogrevOff()
{
  http.begin("http://192.168.0.18/kotel/off");
  http.addHeader("Content-Type", "text/plain");
  if (http.GET() <= 0)
  {
    http.end();
    return;
  }
  http.end();
  podogrev = LOW;
}
