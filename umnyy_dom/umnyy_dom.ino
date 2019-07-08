#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
const char* ssid = "KALYASHINY";
const char* password = "09061995";
ESP8266WebServer server(80);
IPAddress ip(192,168,0,17);  //статический IP
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
bool val1 = LOW;
bool val2 = LOW;
const int led1 = D6;
const int led2 = D7;
void handleRoot() {
  String s = "<h1>Свет в спальне ";
  s+= (val1) ? "включен" : "выключен";
  s+= "</h1>";
  s += "<h2><a href=\"/led1/on\">Включить</a> ";
  s += "<a href=\"/led1/off\">Выключить</a></h2>";
  s += "<h1>Свет в кабинете ";
  s+= (val2) ? "включен" : "выключен";
  s+= "</h1>";
  s += "<h2><a href=\"/led2/on\">Включить</a> ";
  s += "<a href=\"/led2/off\">Выключить</a></h2>";
  server.send(200, "text/html; charset=utf-8", s);
}

// Метод включения диода
void ledOn(int x) {
  if(x ==1)
  {
  val1=HIGH;
  digitalWrite(led1, val1);
  }
  if(x==2)
  {
     val2=HIGH;
  digitalWrite(led2, val2);}
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
   if(x ==1)
  {
    val1=LOW;
  digitalWrite(led1, val1);
  }
  if(x==2)
  {
     val2=LOW;
  digitalWrite(led2, val2);}
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}
void setup(){
  delay(1000);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1,val1);
  digitalWrite(led2,val2);
  delay(1000);
  Serial.begin(115200);
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
  server.begin();
  Serial.println("HTTP server started");
}
void loop(){
  server.handleClient();
}
