/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники на GitHub: https://github.com/AlexGyver/GyverLamp/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/

  Данная модификация основана на версии 1.4 от gunner47 https://github.com/gunner47/GyverLamp
*/

// Ссылка для менеджера плат:
// https://arduino.esp8266.com/stable/package_esp8266com_index.json

#include "Constants.h"

void setup()
{
  Serial.begin(115200);
  Serial.println();
  //ESP.wdtEnable(WDTO_8S);

  //LOG.printf_P(PSTR("BTN_PIN: %d PULL_MODE: %d HIGH_PULL: %d digitalRead(BTN_PIN): %d chk: %d\n"), BTN_PIN, PULL_MODE, HIGH_PULL, digitalRead(BTN_PIN), (digitalRead(BTN_PIN)+((PULL_MODE==HIGH_PULL)?-1:0)));

  // ЛЕНТА/МАТРИЦА                            - перенесено в начало ради сокращения времени когда экран засвечивается хаотичным узором при включении
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)  /*.setCorrection(TypicalLEDStrip)*/;
  FastLED.setBrightness(BRIGHTNESS);                          // установка яркости
  if (CURRENT_LIMIT > 0){
    FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT); // установка максимального тока БП
  }
  FastLED.clear();                                            // очистка матрицы
  leds[getPixelNumber(0, 1)] = CRGB::Red; FastLED.show(); delay(1000);            // красный светодиод инициализации "Включение в сеть" (второй по порядку)

  #ifdef VERTGAUGE
  GaugeSetup();
  #endif

  //LOG.printf_P(PSTR("BTN_PIN: %d PULL_MODE: %d HIGH_PULL: %d digitalRead(BTN_PIN): %d chk: %d\n"), BTN_PIN, PULL_MODE, HIGH_PULL, digitalRead(BTN_PIN), (digitalRead(BTN_PIN)+((PULL_MODE==HIGH_PULL)?-1:0)));

  // ПИНЫ
  #ifdef MOSFET_PIN                                         // инициализация пина, управляющего MOSFET транзистором в состояние "выключен"
  pinMode(MOSFET_PIN, OUTPUT);
  #ifdef MOSFET_LEVEL
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
  #endif
  #endif

  #ifdef ALARM_PIN                                          // инициализация пина, управляющего будильником в состояние "выключен"
  pinMode(ALARM_PIN, OUTPUT);
  #ifdef ALARM_LEVEL
  digitalWrite(ALARM_PIN, !ALARM_LEVEL);
  #endif
  #endif

  // TELNET
  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  telnetServer.begin();
  for (uint8_t i = 0; i < 100; i++)                         // пауза 10 секунд в отладочном режиме, чтобы успеть подключиться по протоколу telnet до вывода первых сообщений
  {
    handleTelnetClient();
    delay(100);
    ESP.wdtFeed();
  }
  #endif

  // КНОПКА
  #ifdef ESP_USE_BUTTON
  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
  touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
  touch.setTimeout(BUTTON_TIMEOUT);
    #if ESP_RESET_ON_START
    leds[getPixelNumber(0, 2)] = CRGB::Green; delay(1); FastLED.show();             // зеленый светодиод инициализации - кнопка присутствует (третий светодиод)
    buttonTick();
    delay(1000);                                            // ожидание инициализации модуля кнопки ttp223 (по спецификации 250мс)
    
    //if (digitalRead(BTN_PIN)+((PULL_MODE==HIGH_PULL)?-1:0)) // если кнопка нажата то будет осуществлена конфигурация
    if (touch.state()) {
      leds[getPixelNumber(0, 3)] = CRGB::Blue; delay(1); FastLED.show();           // синий светодиод инициализации - сброс WIFI (четвертый светодиод)
      wifiManager.resetSettings();                          // сброс сохранённых SSID и пароля при старте с зажатой кнопкой, если разрешено
      #ifdef GENERAL_DEBUG
      LOG.println(F("Init: Настройки WiFiManager сброшены"));
      #endif
      needconfigure = true;                                 // флаг на запуск конфигурации 
      buttonEnabled = true;                                 // при сбросе параметров WiFi сразу после старта с зажатой кнопкой, также разблокируется кнопка, если была заблокирована раньше
      EepromManager::SaveButtonEnabled(&buttonEnabled);
      delay(1000);
    }
    ESP.wdtFeed();
    #endif
  #endif

  // EEPROM
  EepromManager::InitEepromSettings(                        // инициализация EEPROM; запись начального состояния настроек, если их там ещё нет; инициализация настроек лампы значениями из EEPROM
    modes, alarms, &espMode, &ONflag, &dawnMode, &currentMode, &buttonEnabled,
    &(FavoritesManager::ReadFavoritesFromEeprom),
    &(FavoritesManager::SaveFavoritesToEeprom), &GlobalBrightness);
  #ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("Рабочий режим лампы: ESP_MODE = %d\n"), espMode);
  #endif
  
  // WI-FI
  wifiManager.setDebugOutput(WIFIMAN_DEBUG);                // вывод отладочных сообщений
  wifiManager.setMinimumSignalQuality();                    // установка минимально приемлемого уровня сигнала WiFi сетей (8% по умолчанию)
  CaptivePortalManager *captivePortalManager = new CaptivePortalManager(&wifiManager);
  if (espMode == 0U) {                                                                      // если выбран режим WiFi точки доступа
      // wifiManager.setConfigPortalBlocking(false);
    if (sizeof(AP_STATIC_IP)) {
      #ifdef GENERAL_DEBUG
      LOG.println(F("Init: Используется статический IP адрес WiFi точки доступа"));
      #endif
      wifiManager.setAPStaticIPConfig(                                                      // wifiManager.startConfigPortal использовать нельзя, т.к. он блокирует вычислительный процесс внутри себя, а затем перезагружает ESP, т.е. предназначен только для ввода SSID и пароля
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]),      // IP адрес WiFi точки доступа
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], 1),                    // первый доступный IP адрес сети
        IPAddress(255, 255, 255, 0));                                                       // маска подсети
        WiFi.softAP(AP_NAME, AP_PASS);                                                      // конфигурация WiFi в режиме точки доступа
        #ifdef GENERAL_DEBUG
        LOG.println(F("Init: Старт в режиме WiFi точки доступа"));                          // 
        LOG.print(F("Init: IP адрес: "));
        LOG.println(WiFi.softAPIP());
        #endif
        wifiServer.begin();                                                                 // запуск сервера
    }
  }
  else {                                                                                    // режим WiFi клиента 
    FastLED.clear(); delay(1); FastLED.show();                                              // очистка матрицы
    int attempts = 1; float power = 0;
    #ifdef GENERAL_DEBUG
    LOG.println(F("Init: Старт в режиме WiFi клиента (подключение к роутеру)"));
    #endif
    wifiManager.setConnectTimeout(ESP_CONN_TIMEOUT);                                        // установка времени ожидания подключения к WiFi сети, затем старт WiFi точки доступа
    if (WiFi.SSID().length() ){                                                             // подключаемся к роутеру, если есть уже сохранённые SSID и пароль (было подключение к сети)
      ESP.wdtDisable(); delay (100); ESP.wdtEnable(60);  delay(100); 
      WiFi.mode(WIFI_STA);                                                                  // установка режима клиента WiFi     
      WiFi.begin(); delay(2000);                                                            // запуск подключения со старыми параметрами
      #ifdef GENERAL_DEBUG
      LOG.printf_P(PSTR("Init: Подключение к WiFi сети: %s\n"), WiFi.SSID().c_str());
      #endif
      while (WiFi.status() != WL_CONNECTED) {                                               // проверка на устойчивое подключеное соединение
        leds[getPixelNumber(0, attempts)] = CRGB(0, 0, 8 + attempts * 8); delay(1); FastLED.show();           // отображение уровня мощности n
        if (power > 20.5) {
          power == 20.5; // макс +20,5 дБм
        }
      #ifdef GENERAL_DEBUG
      LOG.print("WiFi: Мощность передачи: "); LOG.print(power); LOG.println("dBm ");
      #endif

        WiFi.setOutputPower(power);                                                         // установка мощности передатчика
        power += 1.3667; attempts++;                                                        // увеличение мощности для следующего шага
        delay(1000);                                                                        // задержка перед увеличением мощности
        if (attempts > 15){
          isWifiOffMode = true;
          showWarning(CRGB::Red, 3000U, 500U);                                              // мигание красным цветом 0,5 секунды (1 раз) - не удалось подключится к WiFi сети, продолжаем без WiFi
          #ifdef GENERAL_DEBUG
          LOG.println(""); LOG.println("Init: Подключение не выполнено. Работа без сети.");
          #endif
          WiFi.mode(WIFI_OFF);                                                              // выключить WiFi - работа лампы без сети
        }
      }
    }
    else {                                                
      if (!&buttonEnabled || needconfigure){                                                // Проверка. Требуется ли дополнительная конфигурация?
        if (WiFi.SSID().length()== 0){                                                      // если настройки были сброшены, то запускается процедура подключения через AP
          ESP.wdtDisable(); delay (100); ESP.wdtEnable(ESP_CONF_TIMEOUT * 2);               // установка дежурного счетчика на заведомо большее время ввода конфигурации
          FastLED.clear(); FastLED.setBrightness(BRIGHTNESS); fillAll(CRGB(0,0,32)); delay(1); FastLED.show();   // вся матрица темно синяя = режим конфигурации        
          #ifdef GENERAL_DEBUG
          LOG.println(F("Init: Старт автоконфигурации..."));
          #endif
          CaptivePortalManager *captivePortalManager = new CaptivePortalManager(&wifiManager);
          wifiManager.setConfigPortalTimeout(ESP_CONF_TIMEOUT);                             // установка времени работы WiFi точки доступа, можно боллее 8 секунд - стандартного значениея1
          wifiManager.setBreakAfterConfig(true);  
          wifiManager.autoConnect(host_name.c_str(), AP_PASS);                              // пытаемся подключиться к сохранённой ранее WiFi сети; в случае ошибки, будет развёрнута WiFi точка доступа с указанными AP_NAME и паролем на время ESP_CONN_TIMEOUT секунд; http://AP_STATIC_IP:ESP_HTTP_PORT (обычно http://192.168.0.1:80) - страница для ввода SSID и пароля от WiFi сети роутера
          #ifdef GENERAL_DEBUG
          LOG.println(F("Init: Автоконфигурация звершена"));
          #endif
          FastLED.clear(); delay(1); FastLED.show();                                        // очистка матрицы
          String ssid = WiFi.SSID();                                                                                // запоминание последнего введеного ID сети 
          String pass = WiFi.psk();                                                                                 // запоминание последнего введеного пароля
          int ch = WiFi.channel();                                                                                  // запоминаем номер канала, на который подключался ESP 
          #ifdef GENERAL_DEBUG
          LOG.print("SSID: "); LOG.print(WiFi.SSID()); LOG.print(" "); LOG.println(WiFi.SSID().length());           // показать имя текущей сети и его длину в символах  
          LOG.print("Введенный пароль: "); LOG.print(WiFi.psk()); LOG.print(" "); LOG.println(WiFi.psk().length()); // показать прроль текущей сети и его длину в символах
          LOG.print("Установленный канал при последней попытке соединения: "); LOG.println(ch);                     // показать номер текушего канала  
          #endif
          delay(2000);
          if (ssid == ""){                                                                                          // сеть не выбрана, пароль не введен 
            wifiManager.resetSettings();                                                                            // обнуление настроек WiFi
            if (WiFi.status() == WL_CONNECT_FAILED){                                                                // если была неудачная попытка входа в сеть (сеть прусутствует, но роутер по какой-то причине не принял клиента) 
              showWarning(CRGB::Yellow, 3000U, 500U);                                                                 // мигание желтым цветом 0,5 секунды (3 раза) - сеть WiFi найдена, но вход в сеть не осуществлен
              #ifdef GENERAL_DEBUG
              LOG.println(F("Init: Ошибка входа в сеть!")); delay(2000);
              #endif
            } else {
              showWarning(CRGB::Orange, 3000U, 500U);                                                                 // мигание оранжевым цветом 0,5 секунды (3 раза) - сеть не выбрана
              #ifdef GENERAL_DEBUG
              LOG.println(F("Init: Cеть не выбрана, пароль не введен (((")); delay(2000);
              #endif
              ESP.restart();                                                                                          // перезагрузка
            }
          }

          delete captivePortalManager;                                                                              // сброс настроек всплывающего экрана
          captivePortalManager = NULL;                                                                              //
          #ifdef GENERAL_DEBUG
          LOG.println(); LOG.println("** Сканирование близлежайших сетей **");                                      // Получение списка ближайших сетей
          #endif
          int numSsid = WiFi.scanNetworks();
          if (numSsid == -1){
            LOG.println("Init: Нет доступных сетей! Выход из конфигурации. Сброс.");
            wifiManager.resetSettings();                                                                            // обнуление настроек WiFi
            showWarning(CRGB::Magenta, 3000U, 500U);                                                                // мигание фиолетовым цветом 0,5 секунды (3 раза) - не найдено ни одной WiFi сети
            #ifdef GENERAL_DEBUG
            LOG.println(F("Init: Ошибка входа в сеть..."));
            delay(2000);
            #endif
            ESP.restart();  
          }
          #ifdef GENERAL_DEBUG
          LOG.print("Init: Количество обнаруженных сетей: ");                                                       // вывод количества обнаруженных сетей
          LOG.println(numSsid);
          #endif
          int thisNet = 0;
          for (thisNet = 0; thisNet < numSsid; thisNet++) {                                                         // вывод листинга обнаруженных сетей
            #ifdef GENERAL_DEBUG
            LOG.print(thisNet + 1);
            LOG.print(") ");
            LOG.print("Уровень сигнала: "); LOG.print(WiFi.RSSI(thisNet)); LOG.print("dBm");
            LOG.print("\tКанал: "); LOG.print(WiFi.channel(thisNet));
            LOG.print("\t\tSSID: "); LOG.println(WiFi.SSID(thisNet));
            #endif
            if (ssid == WiFi.SSID(thisNet)){
              ch = WiFi.channel(thisNet);
              #ifdef GENERAL_DEBUG
              LOG.print("Init: Данная сеть наботает на канале: "); LOG.println(ch);
              #endif
            }
            #ifdef GENERAL_DEBUG
            LOG.flush();
            #endif
          }
          #ifdef GENERAL_DEBUG
          LOG.println();
          LOG.println(F("Init: Диагностика подключения..."));                                                       // если конфигурация устройства не прошла 
          #endif
          FastLED.clear(); FastLED.setBrightness(BRIGHTNESS); fillAll(CRGB(16, 0, 0)); delay(1); FastLED.show();    // вся матрица темно красная = режим аварийной конфигурации
  
          if (WiFi.status() != WL_CONNECTED){                                                                       // если подключение к WiFi не установлено
            #ifdef GENERAL_DEBUG
            LOG.println(F("Init: Попытка резервного запуса сети..."));  
            #endif
            delay(2000);
            attempts = 1; power = 0;
            while (WiFi.status() != WL_CONNECTED){                                                                  // старт цикла попыток подключения
              leds[getPixelNumber(0, attempts)] = CRGB(32,32,32); delay(1); FastLED.show();                         // отображение инициализации попытки n
              int myNet;
              numSsid = WiFi.scanNetworks();                                                                        // сканирование сетей на присутствие требуемой
              for (thisNet = 0; thisNet < numSsid; thisNet++) {
                if (ssid == WiFi.SSID(thisNet)){myNet = thisNet;}                                                   // если канал не совпадает с ранее подключавшимся, запомнить новое значение
              }
              #ifdef GENERAL_DEBUG
              LOG.println();
              LOG.print("Канал сети: "); LOG.print(WiFi.SSID(myNet));                                               // вывод параметров следующей попытки
              LOG.print(" Сигнал роутера: "); LOG.print(WiFi.RSSI(myNet)); LOG.println("dBm ");
              LOG.print("Попытка: "); LOG.print(attempts);
              #endif
              WiFi.mode(WIFI_OFF); delay(3000);                                                                     // отключение модуля WiFi/пауза
              leds[getPixelNumber(0, attempts)] = CRGB(32,32,0); delay(1); FastLED.show();                          // отображение режима попытки n
              WiFi.mode(WIFI_STA);                                                                                  // включение модуля WiFi в режиме клиента
              if (power > 20.5) {power == 20.5;}                                                                    // ограничение уровня мощности передатчика. макс 20,5 дБм
              WiFi.setOutputPower(power);
              #ifdef GENERAL_DEBUG
              LOG.print("WiFi: Мощность передачи: "); LOG.print(power); LOG.print("dBm ");
              #endif
              WiFi.begin(ssid, pass);                                                                               // попытка входа в сеть
              #ifdef GENERAL_DEBUG
              LOG.print("WiFi: Статус: "); LOG.print(enumConnectionStatus[WiFi.status()]);                          // вывод статуса подключения
              LOG.print(" Канал: "); LOG.print(WiFi.channel());                                                     // вывод канала подключения 
              LOG.print(" Сигнал: "); LOG.print(WiFi.RSSI()); LOG.print("dBm ");                                    // вывод уровня сигнала
              #endif
              int awaiting = 0;
              while (WiFi.status() != WL_CONNECTED && awaiting < 30){                                               // цикл подключения                                   
                if ((WiFi.channel() == ch && WiFi.RSSI() != 31) || WiFi.status() == WL_IDLE_STATUS){                // Номер канала корректен, Есть сигнал с роутера (в дБм, 31 = отсутствие сигнала), Есть режим подключения
                  LOG.print(".");                                                                                   // точка ожидания - ждем соединения
                }
                else {
                  LOG.print("*");                                                                                   // звездочка ожидания - ждем подключения
                  if (awaiting > 7){awaiting = 100;}
                }
                awaiting++; delay(1000);
              }
              attempts++; power += 1.3667;                                                                          // Увеличить мощность передатчика на 1/15
              LOG.println();
              if (attempts > 15){
                wifiManager.resetSettings();                                                                        // сброс ранее введенных настроек
                showWarning(CRGB::Red, 3000U, 500U);                                                                // мигание красным цветом 0,5 секунды (3 раза) - не удалось подключится к WiFi сети, перезагрузка со сбросом настроек
                #ifdef GENERAL_DEBUG
                LOG.println(""); LOG.println("Init: Соединение не выполнено! Перезагрузка...");
                #endif
                ESP.restart();                                                                                      // перезагркзка
              }
            }
          }
          #ifdef GENERAL_DEBUG
          LOG.print(F("IP адрес: ")); LOG.println(WiFi.localIP()); 
          #endif
          WiFi.persistent(true);                                                                                    // сохранение настроек
          showWarning(CRGB::Green, 3000U, 500U);                                                                    // мигание зеленым цветом 0,5 секунды (3 раза) - успешный вход в WiFi сеть, перезагрузка для применения настроек
          #ifdef GENERAL_DEBUG
          LOG.println(F("Init: Рестарт для применения заданного статического IP адреса..."));
          #endif
          delay(100); ESP.restart();                                                                                // задержка/перезагркзка
        } 
      }
      else {                                                                                                        // Если сеть не была настроена ранее. Работа без WiFi
        isWifiOffMode = true;
        #ifdef GENERAL_DEBUG
        LOG.println(F("Init: Работа без WiFi сети..."));
        #endif
        showWarning(CRGB::White, 1000U, 500U);                                                                      // мигание белым цветом 0,5 секунды (1 раз) - режим работы без сети
        WiFi.mode(WIFI_OFF);                                                                                        // включение лампы без WI-Fi
      }
    }
    // меняем IP адрес на предустановленный, если он определен

    if (sizeof(STA_STATIC_IP) && WiFi.localIP() != IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3])){  // ВНИМАНИЕ: настраивать статический ip WiFi клиента можно только при уже сохранённых имени и пароле WiFi сети (иначе проявляется несовместимость библиотек WiFiManager и WiFi)
      #ifdef GENERAL_DEBUG
      LOG.print(F("Init: Сконфигурирован статический IP адрес: "));
      LOG.printf_P(PSTR("%u.%u.%u.%u\n"), STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]);
      #endif
      wifiManager.setSTAStaticIPConfig(
        IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], STA_STATIC_IP[3]),      // статический IP адрес ESP в режиме WiFi клиента
        IPAddress(STA_STATIC_IP[0], STA_STATIC_IP[1], STA_STATIC_IP[2], 1),                     // первый доступный IP адрес сети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
        IPAddress(255, 255, 255, 0));                                                           // маска подсети (справедливо для 99,99% случаев; для сетей меньше чем на 255 адресов нужно вынести в константы)
    }
  }
  
  #ifdef GENERAL_DEBUG
  LOG.print(F("IP адрес: ")); LOG.println(WiFi.localIP());
  LOG.printf_P(PSTR("Порт UDP сервера: %u\n"), localPort);
  #endif
  Udp.begin(localPort);                                                                         // конфигурация UDP порта
  ESP.wdtDisable(); delay (100); ESP.wdtEnable(WDTO_8S);                                      // запуск ждущего таймера на 8 секунд
  delay (100);

  // NTP
  #ifdef USE_NTP
  timeClient.begin();
  //ESP.wdtFeed();
  #endif


  // MQTT
  #ifdef USE_MQTT
  if (espMode == 1U)
  {
    mqttClient = new AsyncMqttClient();
    MqttManager::setupMqtt(mqttClient, inputBuffer, &sendCurrent);    // создание экземпляров объектов для работы с MQTT, их инициализация и подключение к MQTT брокеру
  }
  //ESP.wdtFeed();
  #endif


  // ОСТАЛЬНОЕ
  memset(matrixValue, 0, sizeof(matrixValue));
  randomSeed(micros());
  changePower();
  loadingFlag = true;

  //lampMode = MODE_DEMO; // для тестирования и отладки :) включаю принудительно в демо
  #ifndef ESP_USE_BUTTON
  if(isWifiOffMode)
    lampMode = MODE_DEMO; // запуск в демо-режиме если используется без кнопки и без сети
  #endif
}


void loop()
{
  parseUDP();
 
  effectsTick();

  EepromManager::HandleEepromTick(&settChanged, &eepromTimeout, &ONflag, 
    &currentMode, modes, &(FavoritesManager::SaveFavoritesToEeprom));

  #ifdef ESP_USE_BUTTON
  if (buttonEnabled)
  {
    buttonTick();
  }
  #endif

  #ifdef OTA
  otaManager.HandleOtaUpdate();                             // ожидание и обработка команды на обновление прошивки по воздуху
  #endif

  TimerManager::HandleTimer(&ONflag, &settChanged,          // обработка событий таймера отключения лампы
    &eepromTimeout, &changePower);

  if (FavoritesManager::HandleFavorites(                    // обработка режима избранных эффектов
      &ONflag,
      &currentMode,
      &loadingFlag
      #ifdef USE_NTP
      , &dawnFlag
      #endif
      ))
  {
    FastLED.setBrightness(modes[currentMode].Brightness);
    FastLED.clear();
  }

  #ifdef USE_MQTT
  if (espMode == 1U && mqttClient && WiFi.isConnected() && !mqttClient->connected())
  {
    MqttManager::mqttConnect();                             // библиотека не умеет восстанавливать соединение в случае потери подключения к MQTT брокеру, нужно управлять этим явно
    MqttManager::needToPublish = true;
  }

  if (MqttManager::needToPublish)
  {
    if (strlen(inputBuffer) > 0)                            // проверка входящего MQTT сообщения; если оно не пустое - выполнение команды из него и формирование MQTT ответа
    {
      processInputBuffer(inputBuffer, MqttManager::mqttBuffer, true);
    }
    
    MqttManager::publishState();
  }
  #endif

  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  handleTelnetClient();
  #endif
  
  delay(1);
  ESP.wdtFeed();                                            // пнуть собаку
}
