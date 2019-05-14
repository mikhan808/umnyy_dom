void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, HIGH);   // зажигаем светодиод
  delay(1000);              // ждем секунду
  digitalWrite(13, LOW);    // выключаем светодиод
  delay(1000);              // ждем секунду

}
