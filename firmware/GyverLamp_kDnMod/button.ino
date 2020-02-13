#ifdef ESP_USE_BUTTON

#ifdef GENERAL_DEBUG
void debugPrint(){
    LOG.printf_P(PSTR("lampMode: %d numHold: %d currentMode: %d brightness: %d speed: %d scale: %d\n"), lampMode, numHold, currentMode, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale);
}
#endif

void changeDirection(byte numHold){
  if(!startButtonHolding || (tmChangeDirectionTimer.isReady() && setDirectionTimeout)){
    switch(numHold){
      case 1: brightDirection = !brightDirection; break;
      case 2: speedDirection = !speedDirection; break;
      case 3: scaleDirection = !scaleDirection; break;
    }
    setDirectionTimeout = false;
  }
  #ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("changeDirection %d, %d, %d\n"), brightDirection, speedDirection, scaleDirection);
  #endif
}

void buttonTick()
{
  if (!buttonEnabled)                                       // события кнопки не обрабатываются, если она заблокирована
  {
    return;
  }

  touch.tick();

  if (!ONflag) { // Обработка из выключенного состояния
    if (touch.isDouble()) { // Демо-режим, с переключением каждые 30 секунд для двойного клика в выключенном состоянии
      lampMode = MODE_DEMO;
      randomSeed(millis());
      currentMode = random(0, MODE_AMOUNT)%EFF_WHITE_COLOR; // EFF_WHITE_COLOR и выше скипаем
      GlobalBrightness = EEPROM.read(EEPROM_GLOBAL_BRIGHTNESS);
      delay(5);
      if(lampMode==MODE_DEMO && GlobalBrightness>0)
        FastLED.setBrightness(GlobalBrightness);
      else
        FastLED.setBrightness(modes[currentMode].Brightness);
      ONflag = true;
      tmDemoTimer.reset(); // момент включения для таймаута в DEMOTIME
      changePower();
      #ifdef GENERAL_DEBUG
      LOG.printf_P(PSTR("Demo mode: %d, storeEffect: %d\n"), currentMode, storeEffect);
      #endif
    }
    
    if (touch.isHolded()) {
      #ifdef GENERAL_DEBUG
      LOG.printf_P(PSTR("Удержание кнопки из выключенного состояния\n"));
      #endif
      // Включаем белую лампу в полную яркость
      numHold = 1;
      lampMode = MODE_WHITELAMP;
      currentMode = EFF_WHITE_COLOR;
      modes[currentMode].Brightness = BRIGHTNESS;
      FastLED.setBrightness(modes[currentMode].Brightness);

      #ifdef GENERAL_DEBUG
      LOG.printf_P(PSTR("currentMode: %d, storeEffect: %d\n"), currentMode, storeEffect);
      #endif
      
      ONflag = true;
      startButtonHolding = true;
      setDirectionTimeout = false;
      tmNumHoldTimer.reset();
      tmChangeDirectionTimer.reset();
      brightDirection = 1;
      changePower();
    }
  } 

  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;

  // однократное нажатие
  if (clickCount == 1U)
  {
    if(!ONflag){
      numHold = 0;
      lampMode = MODE_NORMAL;
      currentMode = storeEffect;
    } else {
      storeEffect = ((currentMode == EFF_WHITE_COLOR) ? storeEffect : currentMode); // сохраняем предыдущий эффект, если только это не белая лампа
    }
    
    #ifdef GENERAL_DEBUG
    if(ONflag)
      LOG.printf_P(PSTR("Лампа выключена, currentMode: %d, storeEffect: %d\n"), currentMode, storeEffect);
    else
      LOG.printf_P(PSTR("Лампа включена, currentMode: %d, storeEffect: %d\n"), currentMode, storeEffect);
    #endif
    
    if (dawnFlag)
    {
      manualOff = true;
      dawnFlag = false;
      FastLED.setBrightness(modes[currentMode].Brightness);
      changePower();
    }
    else
    {
      ONflag = !ONflag;
      changePower();
    }
    settChanged = true;
    eepromTimeout = millis();
    loadingFlag = true;

    #ifdef USE_MQTT
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
  }


  // двухкратное нажатие
  if (ONflag && clickCount == 2U)
  {
    if (++currentMode >= (int8_t)MODE_AMOUNT) currentMode = 0;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);

    #ifdef USE_MQTT
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
  }


  // трёхкратное нажатие
  if (ONflag && clickCount == 3U)
  {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    FastLED.clear();
    delay(1);

    #ifdef USE_MQTT
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
  }


  // четырёхкратное нажатие
  if (clickCount == 4U)
  {
    #ifdef OTA
    if (otaManager.RequestOtaUpdate())
    {
      ONflag = true;
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
      FastLED.clear();
      delay(1);
      changePower();
    }
    #endif
  }


  // пятикратное нажатие
  if (clickCount == 5U)                                     // вывод IP на лампу
  {
    if (espMode == 1U)
    {
      loadingFlag = true;
      
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
      digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
      #endif

      while(!fillString(WiFi.localIP().toString().c_str(), CRGB::White)) { delay(1); ESP.wdtFeed(); }

      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
      digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
      #endif

      loadingFlag = true;
    }
  }


  // шестикратное нажатие
  if (clickCount == 6U)                                     // вывод текущего времени бегущей строкой
  {
    printTime(thisTime, true, ONflag, false, false);
  }


  // семикратное нажатие
  if (ONflag && clickCount == 7U)                           // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот
  {
    espMode = (espMode == 0U) ? 1U : 0U;
    EepromManager::SaveEspMode(&espMode);

    #ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\nНовый рабочий режим: ESP_MODE = %d, %s\nРестарт...\n"),
      espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
    #endif

    showWarning(CRGB::Red, 3000U, 500U);                    // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }

  // кнопка только начала удерживаться
  if (ONflag && touch.isHolded()){ //  && lampMode != MODE_WHITELAMP
    startButtonHolding = true;
    setDirectionTimeout = false;
    isFirstHoldingPress = true;
    switch (touch.getHoldClicks()){
      case 0U: {
        if(!numHold){
          numHold = 1;
          if(lampMode==MODE_DEMO) {
            storeEffBrightness = modes[currentMode].Brightness; // запоминаем яркость эффекта, которая была изначально
            modes[currentMode].Brightness = GlobalBrightness; // перенакрываем глобальной
          }
        }
        break;
      }
      case 1U: {
        if(!numHold)
          numHold = 2;
        break;
      }
      case 2U: {
        if(!numHold)
          numHold = 3;
        break;
      }
    }
  }

  // кнопка нажата и удерживается
  if (ONflag && touch.isStep())
  {
    if(!isFirstHoldingPress && (((modes[currentMode].Brightness == BRIGHTNESS || modes[currentMode].Brightness <= 1) && numHold == 1)
    || ((modes[currentMode].Speed == 255 || modes[currentMode].Speed <= 1) && numHold == 2)
    || ((modes[currentMode].Scale == 255 || modes[currentMode].Scale <= 1) && numHold == 3))){
      if(!setDirectionTimeout){
        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Граничное значение! numHold: %d brightness: %d speed: %d scale: %d\n"), numHold,modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale);
        #endif
        tmChangeDirectionTimer.reset(); // пауза на смену направления
        setDirectionTimeout = true;
      }
      else {
        changeDirection(numHold);
      }
    }

    // Для второго входа, сбрасываем флаг
    isFirstHoldingPress = false;
    
    #ifdef GENERAL_DEBUG
    debugPrint(); // отладка
    #endif
    if (numHold != 0) {
      tmNumHoldTimer.reset();
      tmDemoTimer.reset(); // сбрасываем таймер переключения, если регулируем яркость/скорость/масштаб
      //loadingFlag = true;
    }
    
    switch (numHold) {
      case 1:
        if(currentMode==EFF_WHITE_COLOR)
          modes[currentMode].Brightness = constrain(modes[currentMode].Brightness + (modes[currentMode].Brightness / 25 + 1) * (brightDirection * 2 - 1), MIN_WHITE_COLOR_BRGHT , BRIGHTNESS);
        else
          modes[currentMode].Brightness = constrain(modes[currentMode].Brightness + (modes[currentMode].Brightness / 25 + 1) * (brightDirection * 2 - 1), 1 , BRIGHTNESS);
        break;

      case 2:
        modes[currentMode].Speed = constrain(modes[currentMode].Speed + (modes[currentMode].Speed / 25 + 1) * (speedDirection * 2 - 1), 1 , 255);
        break;

      case 3:
        modes[currentMode].Scale = constrain(modes[currentMode].Scale + (modes[currentMode].Scale / 25 + 1) * (scaleDirection * 2 - 1), 1 , 255);
        break;
    }

    if(numHold==1){
        FastLED.setBrightness(modes[currentMode].Brightness);
    }
    
    settChanged = true;
    eepromTimeout = millis();
  }

  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding)      // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
  {
    startButtonHolding = false;
    setDirectionTimeout = false;
    loadingFlag = true;

    changeDirection(numHold);

    #ifdef GENERAL_DEBUG
    switch (numHold) {
      case 1:
        LOG.printf_P(PSTR("Новое значение яркости: %d\n"), modes[currentMode].Brightness);
        if(lampMode==MODE_DEMO) {GlobalBrightness = modes[currentMode].Brightness; EepromManager::SaveGlobalBrightness(&GlobalBrightness); modes[currentMode].Brightness = storeEffBrightness;}
        break;
      case 2:
        LOG.printf_P(PSTR("Новое значение скорости: %d\n"), modes[currentMode].Speed);
        break;
      case 3:
        LOG.printf_P(PSTR("Новое значение масштаба: %d\n"), modes[currentMode].Scale);
        break;
    }
    #endif

    #ifdef USE_MQTT
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif

    if(lampMode==MODE_DEMO && GlobalBrightness>0)
      FastLED.setBrightness(GlobalBrightness);
    else
      FastLED.setBrightness(modes[currentMode].Brightness);

    // возвращаем сохраненную яркость, после регулировки глобальной яркости для демо
    if(lampMode == MODE_DEMO)
      modes[currentMode].Brightness = storeEffBrightness;
  }

  if (tmNumHoldTimer.isReadyManual() && !startButtonHolding) { // сброс текущей комбинации в обычном режиме, если уже не нажата
      numHold = 0;
  }
}
#endif
