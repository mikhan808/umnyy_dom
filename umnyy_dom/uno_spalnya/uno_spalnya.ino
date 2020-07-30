const int PIN_SPALNYA_1 = 8;
const int PIN_SPALNYA_2 = 9;
const int PIN_CABINET_1 = 10;
const int PIN_CABINET_2 = 11;
void setup() {
  Serial.begin(115200);
  pinMode(PIN_SPALNYA_1,OUTPUT);
  pinMode(PIN_SPALNYA_2,OUTPUT);
  pinMode(PIN_CABINET_1,OUTPUT);
  pinMode(PIN_CABINET_2,OUTPUT);

}
String inString;
void loop() {
  
 while (Serial.available() > 0) {

 inString=Serial.readString();
  if(inString=="on_spalnya_1")
  {
    digitalWrite(PIN_SPALNYA_1,HIGH);
    Serial.print("OK");
  }
    if(inString=="on_spalnya_2")
    {
    digitalWrite(PIN_SPALNYA_2,HIGH);
    Serial.print("OK");
  }
    if(inString=="off_spalnya_1")
    {
    digitalWrite(PIN_SPALNYA_1,LOW);
    Serial.print("OK");
  }
    if(inString=="off_spalnya_2")
    {
    digitalWrite(PIN_SPALNYA_2,LOW);
    Serial.print("OK");
  }
    if(inString=="on_cabinet_1")
    {
    digitalWrite(PIN_CABINET_1,HIGH);
    Serial.print("OK");
  }
    if(inString=="on_cabinet_2")
    {
    digitalWrite(PIN_CABINET_2,HIGH);
    Serial.print("OK");
  }
    
   if(inString=="off_cabinet_1")
   {
    digitalWrite(PIN_CABINET_1,LOW);
     Serial.print("OK");
   }
    if(inString=="off_cabinet_2")
    {
    digitalWrite(PIN_CABINET_2,LOW);
     Serial.print("OK");
    }

  }
}
