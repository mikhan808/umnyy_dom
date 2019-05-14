#include "IRremote.h"

IRrecv irrecv(2); // указываем вывод, к которому подключен приемник

decode_results results;
bool svet_spalnya;
int svet_spalnya_pin = 13;

void setup() {
  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием
  svet_spalnya = HIGH;
  pinMode(svet_spalnya_pin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0)
  {
    char pc_code = Serial.read();
    Serial.println( pc_code); // печатаем данные
    if (pc_code == 'h')
    {
      svet_spalnya = HIGH;
      change_status();
    }
    if (pc_code == 'l')
    {
      svet_spalnya = LOW;
      change_status();
    }
  }
  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    if (results.value == 0x2FDA05F)
    {
      if (svet_spalnya == HIGH)
      {
        svet_spalnya = LOW;
      }
      else
      {
        svet_spalnya = HIGH;
      }
      change_status();
    }
    irrecv.resume(); // принимаем следующую команду
  }
}
void change_status()
{
  digitalWrite(svet_spalnya_pin, svet_spalnya);  // зажигаем светодиод
  delay(1000);
}
