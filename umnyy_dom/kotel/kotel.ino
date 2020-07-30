#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* ssid = "********";
const char* password = "********";
ESP8266WebServer server(80);

IPAddress ip(192,168,0,18);  //статический IP
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
bool podogrev;
int pinKotel=2;
int pinDavlenie=0;

void handleRoot() {
  String s = "<h1>Котел ";
    s+= (podogrev) ? "включен" : "выключен";
    s+= "</h1>";
    s += "<h2><a href=\"/kotel/on\">Включить</a> ";
    s += "<a href=\"/kotel/off\">Выключить</a></h2>";
    s += "<h2><a href=\"/davlenie\">Прибавить давление</a> ";
    s += "</h2>";
  server.send(200, "text/html; charset=utf-8", s);
}
void kotelOn()
{
   podogrev=HIGH;
   digitalWrite(pinKotel, podogrev);
   server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void kotelOff()
{
   podogrev=LOW;
   digitalWrite(pinKotel, podogrev);
   server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void davlenie()
{
   digitalWrite(pinDavlenie, HIGH);
   delay(3000);
   digitalWrite(pinDavlenie, LOW);
   server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}
 
void setup(){
  pinMode(pinDavlenie, OUTPUT);
  digitalWrite(pinDavlenie,LOW);
  pinMode(pinKotel, OUTPUT);
  delay(1000);
  digitalWrite(pinKotel,podogrev);
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
  server.on("/kotel/on", kotelOn);
  server.on("/kotel/off", kotelOff);
  server.on("/davlenie", davlenie);
  server.begin();
  Serial.println("HTTP server started");
}
void loop(){
  server.handleClient();
}
