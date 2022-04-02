#include <ESP8266WiFi.h>
#include<FS.h>
#include<ArduinoJson.h>
#include "DHT.h"
#include "Parser.h"
#include "AsyncStream.h"  // асинхронное чтение сериал
WiFiClient client;
AsyncStream<50> serial(&client, ';');  
char* host = "192.168.0.40";
uint16_t port = 100;



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
  Off = 0,
  One = 1,
  Two = 2,
  All = 3
};
Status_Svet Spalnya_status;
Status_Svet Cabinet_status;
bool podogrev = LOW;

DHT dht(DHTPIN, DHTTYPE);

String jsonConfig = "{}";
float currentTemperature;
float currentHumidity;
long currentSensor;
unsigned long lastRequestTemperature;
String build_string_status(Status_Svet st)
{
  String res = "";
  if (st == Off)
    res = "Выключен";
  if (st == One)
    res = "Включен 1";
  if (st == Two)
    res = "Включен 2";
  if (st == All)
    res = "Включен Все";
  return res;
}
int build_int_status(Status_Svet st)
{
  if (st == Off)
    return 0;
  if (st == One)
    return 1;
  if (st == Two)
    return 2;
  if (st == All)
    return 3;
  return 0;
}

void change_value(String type, int inc) {
  if (type == "lustra")
  {
    lustra = inc;
  }
  saveConfig();
//redirectToHome();
}
void rezhim_lustra()
{
  change_value("lustra", !lustra);
}

Status_Svet int_to_Status_Svet(int x)
{
  if (x == 0)
    return Off;
  if (x == 1)
    return One;
  if (x == 2)
    return Two;
  if (x == 3)
    return All;
}
Status_Svet switch_status(Status_Svet st)
{
  if (st == Off)
  {
    if (!lustra)
      return All;
    return One;
  }
  if (st == One)
  {
    if (!lustra)
      return Off;
    return Two;
  }
  if (st == Two)
  {
    if (!lustra)
      return Off;
    return All;
  }
  if (st == All)
    return Off;
}
Status_Svet switch_status(Status_Svet st, bool on, int number)
{
  if (on)
  {
    if (st == Off || st == int_to_Status_Svet(number))
    {
      return int_to_Status_Svet(number);
    }
    else return All;
  } else
  {
    if (st == All || int_to_Status_Svet(All - number) == st)
    {
      return int_to_Status_Svet(All - number);
    }
    else return Off;
  }
}

void sendInfoToUno(String room, Status_Svet st)
{
  if (st == Off)
  {
    if (room == "spalnya")
    {
      digitalWrite(PIN_SVET_SPALNYA_1, LOW);
      digitalWrite(PIN_SVET_SPALNYA_2, LOW);
    }
    else
    {
      digitalWrite(PIN_SVET_CABINET_1, LOW);
      digitalWrite(PIN_SVET_CABINET_2, LOW);
    }
  }
  if (st == One)
  {
    if (room == "spalnya")
    {
      digitalWrite(PIN_SVET_SPALNYA_1, HIGH);
      digitalWrite(PIN_SVET_SPALNYA_2, LOW);
    }
    else
    {
      digitalWrite(PIN_SVET_CABINET_1, HIGH);
      digitalWrite(PIN_SVET_CABINET_2, LOW);
    }
  }
  if (st == Two)
  {
    if (room == "spalnya")
    {
      digitalWrite(PIN_SVET_SPALNYA_1, LOW);
      digitalWrite(PIN_SVET_SPALNYA_2, HIGH);
    }
    else
    {
      digitalWrite(PIN_SVET_CABINET_1, LOW);
      digitalWrite(PIN_SVET_CABINET_2, HIGH);
    }
  }
  if (st == All)
  {
    if (room == "spalnya")
    {
      digitalWrite(PIN_SVET_SPALNYA_1, HIGH);
      digitalWrite(PIN_SVET_SPALNYA_2, HIGH);
    }
    else
    {
      digitalWrite(PIN_SVET_CABINET_1, HIGH);
      digitalWrite(PIN_SVET_CABINET_2, HIGH);
    }
  }

}

// Метод включения диода
void ledOn(int x, int number) {
  if (x == 1)
  {
    Spalnya_status = switch_status(Spalnya_status, true, number);
    sendInfoToUno("spalnya", Spalnya_status);
  }
  if (x == 2)
  {
    Cabinet_status = switch_status(Cabinet_status, true, number);
    sendInfoToUno("cabinet", Cabinet_status);
  }
  //redirectToHome();
}
void ledOff(int x, int number) {
  if (x == 1)
  {
    Spalnya_status = switch_status(Spalnya_status, false, number);
    sendInfoToUno("spalnya", Spalnya_status);
  }
  if (x == 2)
  {
    Cabinet_status = switch_status(Cabinet_status, false, number);
    sendInfoToUno("cabinet", Cabinet_status);
  }
  //redirectToHome();
}

// Метод выключения диода
void led1on1()
{
  ledOn(1, 1);
}
void led1on2()
{
  ledOn(1, 2);
}
void led2on1()
{
  ledOn(2, 1);
}
void led2on2()
{
  ledOn(2, 2);
}
void led1off1()
{
  ledOff(1, 1);
}
void led1off2()
{
  ledOff(1, 2);
}
void led2off1()
{
  ledOff(2, 1);
}
void led2off2()
{
  ledOff(2, 2);
}

void checkButtons()
{
  if (digitalRead(PIN_BUTTON_SPALNYA) == HIGH)
  {
    Spalnya_status = switch_status(Spalnya_status);
    sendInfoToUno("spalnya", Spalnya_status);
  }
  if (digitalRead(PIN_BUTTON_CABINET) == HIGH)
  {
    Cabinet_status = switch_status(Cabinet_status);
    sendInfoToUno("cabinet", Cabinet_status);
  }
}

void setup() {
  setupOTA();
  delay(1000);
  pinMode(PIN_BUTTON_SPALNYA, INPUT);
  pinMode(PIN_BUTTON_CABINET, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(PIN_SVET_SPALNYA_1, OUTPUT);
  pinMode(PIN_SVET_SPALNYA_2, OUTPUT);
  pinMode(PIN_SVET_CABINET_1, OUTPUT);
  pinMode(PIN_SVET_CABINET_2, OUTPUT);
  Spalnya_status = Off;
  Cabinet_status = Off;
  bool diod = true;
  delay(1000);
  dht.begin();
  SPIFFS.begin();
  digitalWrite(BUILTIN_LED, true);
  //setupWebServer();


}
unsigned long lastCheckButtons = 0;
void loop() {
  loopOTA();
  if(client.connected())
  {
    sendData();
    parsing();
  }
  else
  {
    client.connect(host,port);
    
  }
  delay(1000);
 
}

void sendData()
{
   loadConfig();
  //spalnya
  client.print(1);
  client.print(',');
  client.print(build_int_status(Spalnya_status));
  client.println(';');
  //detskaya
  client.print(2);
  client.print(',');
  client.print(build_int_status(Cabinet_status));
  client.println(';');
  //temperature
  client.print(3);
  client.print(',');
  client.print(temperature(),2);
  client.println(';');  
  //humidity()
  client.print(4);
  client.print(',');
  client.print(humidity(),2); 
  client.println(';'); 
}

void parsing() {
  if (serial.available()) {
    Serial.println(serial.buf);
    Parser data(serial.buf, ',');   // // отдаём парсеру
    int ints[10];           // массив для численных данных
    data.parseInts(ints);   // парсим в него
    switch (ints[0]) {
      case 1:
      case 2: 
      if(ints[2]==1)
            ledOn(ints[0],ints[1]);
      else ledOff(ints[0],ints[1]);
      break;
      case 3:rezhim_lustra();
      break;
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
