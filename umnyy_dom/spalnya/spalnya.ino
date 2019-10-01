#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include<FS.h>
#include<ArduinoJson.h>

bool podogrev = LOW;
const char* ssid = "KALYASHINY";
const char* password = "09061995";
ESP8266WebServer server(80);
IPAddress ip(192, 168, 0, 17); //статический IP
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
bool val1 = LOW;
bool val2 = LOW;
const int led1 = D6;
const int led2 = D5;
OneWire oneWire(D7);
DallasTemperature sensors(&oneWire);
HTTPClient http;

float t_zh = 22.0;
float delta = 0.1;
float delta_change = 0.1;
String jsonConfig = "{}";
float currentTemperature;
unsigned long lastRequestTemperature;

void handleRoot() {
  loadConfig();
  String s = "<h1>Свет в спальне ";
  s += (val1) ? "включен" : "выключен";
  s += "</h1>";
  s += "<h2><a href=\"/led1/on\">Включить</a> ";
  s += "<a href=\"/led1/off\">Выключить</a></h2>";
  s += "<h1>Свет в кабинете ";
  s += (val2) ? "включен" : "выключен";
  s += "</h1>";
  s += "<h2><a href=\"/led2/on\">Включить</a> ";
  s += "<a href=\"/led2/off\">Выключить</a></h2>";
  s += "<h1>Температура в спальне ";
  s += String(currentTemperature);
  s += "'C";
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

// Метод включения диода
void ledOn(int x) {
  if (x == 1)
  {
    val1 = HIGH;
    digitalWrite(led1, val1);
  }
  if (x == 2)
  {
    val2 = HIGH;
    digitalWrite(led2, val2);
  }
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

// Метод выключения диода
void led1on()
{
  ledOn(1);
}
void led2on()
{
  ledOn(2);
}
void led1off()
{
  ledOff(1);
}
void led2off()
{
  ledOff(2);
}

void ledOff(int x) {
  if (x == 1)
  {
    val1 = LOW;
    digitalWrite(led1, val1);
  }
  if (x == 2)
  {
    val2 = LOW;
    digitalWrite(led2, val2);
  }
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}
void setup() {
  delay(1000);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, val1);
  digitalWrite(led2, val2);
  delay(1000);
  Serial.begin(115200);
  SPIFFS.begin();
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial.println("");
  // ожидание соединения
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/led1/on", led1on);
  server.on("/led1/off", led1off);
  server.on("/led2/on", led2on);
  server.on("/led2/off", led2off);
  server.on("/delta_change_increase", delta_change_increase);
  server.on("/delta_change_decrease", delta_change_decrease);
  server.on("/temperature_increase", temperature_increase);
  server.on("/temperature_decrease", temperature_decrease);
  server.on("/delta_increase", delta_increase);
  server.on("/delta_decrease", delta_decrease);
  server.begin();
  Serial.println("HTTP server started");
  currentTemperature = temperature();
  lastRequestTemperature = millis();
}
void loop() {
  unsigned long currentTime = millis();
  if (abs(currentTime - lastRequestTemperature) > 2000)
  {
    currentTemperature = temperature();
    lastRequestTemperature = millis();
  }
  server.handleClient();
  if (podogrev)
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
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
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
