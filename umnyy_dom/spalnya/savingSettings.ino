

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
  DynamicJsonDocument jsonBuffer(1024);
  //  вызовите парсер JSON через экземпляр jsonBuffer
  //  строку возьмем из глобальной переменной String jsonConfig
  deserializeJson(jsonBuffer, jsonConfig);
  JsonObject root = jsonBuffer.as<JsonObject>();
  // Теперь можно получить значения из root
  lustra = root["lustra"];
  return true;
}

bool saveConfig() {
  // Резервируем памяь для json обекта буфер может рости по мере необходимти предпочтительно для ESP8266
  DynamicJsonDocument jsonBuffer(1024);
  //  вызовите парсер JSON через экземпляр jsonBuffer
  deserializeJson(jsonBuffer, jsonConfig);
  JsonObject json = jsonBuffer.as<JsonObject>();
  json["lustra"] = lustra;
  // Помещаем созданный json в глобальную переменную json.printTo(jsonConfig);
  serializeJson(json,jsonConfig);
  //json.printTo(jsonConfig);
  // Открываем файл для записи
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  serializeJson(json,configFile);
  // Записываем строку json в файл
  //json.printTo(configFile);
  return true;
}
