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
      digitalWrite(svet_spalnya_pin, svet_spalnya);  // зажигаем светодиод
      delay(1000);
    }
    irrecv.resume(); // принимаем следующую команду
  }
}
