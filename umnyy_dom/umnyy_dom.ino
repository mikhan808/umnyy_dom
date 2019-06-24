#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
const char* ssid = "****";
const char* password = "*****";
IPAddress ip(192,168,0,17);  //статический IP
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);
bool val = LOW;
const int led = D8;
void handleRoot() {
  String s = (val) ? "<h1>LED On</h1>" : "<h1>LED Off</h1>";
  s += "<h2><a href=\"/led/on\">LED On</a> ";
  s += "<a href=\"/led/off\">LED Off</a></h2>";
  server.send(200, "text/html", s);
}

// Метод включения диода
void ledOn() {
  val = HIGH; // Включаем диод
  digitalWrite(led, HIGH);
  Serial.println(val);
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

// Метод выключения диода
void ledOff() {
  val = LOW; // Выключаем диод
  digitalWrite(led, LOW);
  Serial.println(val);
  // Перенаправление обратно на стартовую страницу
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}
void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led,val);
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
  server.on("/led/on", ledOn);
  server.on("/led/off", ledOff);
  server.begin();
  Serial.println("HTTP server started");
}
void loop(void){
  server.handleClient();
}
