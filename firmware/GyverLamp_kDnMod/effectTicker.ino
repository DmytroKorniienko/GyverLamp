uint32_t effTimer;
byte ind;
boolean isFaderOn = false;
byte faderStep = 1;

void effectsTick()
{
  // будильник рассвет будет жить тут
  #ifdef USE_NTP
  timeTick();
  #endif
  
  if (!dawnFlag) // флаг устанавливается будильником рассвет
  {
    if (ONflag)
    {
      if(millis() - effTimer >= modes[currentMode].Speed && !isFaderOn){
        EFFECTS_ARR[currentMode].func(EFFECTS_ARR[currentMode].param);
        effTimer = millis();
      }

      if(!tmFaderTimeout.isReady()){
        if(isFaderOn && tmFaderStepTime.isReady()) {
          #ifdef GENERAL_DEBUG
          //LOG.printf_P(PSTR("leds[1]=%d %d %d\n"),leds[1].red,leds[1].green,leds[1].blue);
          #endif
          faderStep++;
          float chVal = ((float)GlobalBrightness*FADERSTEPTIME)/FADERTIMEOUT;
          for(int led = 0 ; led < NUM_LEDS ; led++ ) {
            //leds[led]/=((faderStep>5)?2:1);
            leds[led].subtractFromRGB((uint8_t)(chVal*faderStep*0.33));
          }
        }
      } else { // время на фейдер вышло
        tmFaderTimeout.setInterval(0); // отключить до следующего раза, также переключаем эффект на новый, заодно запоминаем яркость текущего эффекта
        if(RANDOM_DEMO)
            currentMode = random(0, MODE_AMOUNT)%EFF_WHITE_COLOR; // EFF_WHITE_COLOR скипаем
          else
            currentMode=(currentMode+1)%EFF_WHITE_COLOR; // EFF_WHITE_COLOR скипаем и идем по наростанию
        storeEffBrightness = modes[currentMode].Brightness;
        loadingFlag = true; // некоторые эффекты требуют начальной иницализации, поэтому делаем так...
        isFaderOn = false;
        faderStep = 1;
        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("%s Demo mode: %d, storeEffect: %d\n"),(RANDOM_DEMO?"Random":"Seq") , currentMode, storeEffect);
        #endif
        EFFECTS_ARR[currentMode].func(EFFECTS_ARR[currentMode].param); // отрисовать новый эффект
      }

      #ifdef USELEDBUF
      memcpy(ledsbuff, leds, sizeof(CRGB)* NUM_LEDS);                             // сохранение сформированной картинки эффекта в буфер (для медленных или зависящих от предыдущей)
      #endif

      if(tmDemoTimer.isReady() && (lampMode == MODE_DEMO)){
        tmFaderTimeout.setInterval(FADERTIMEOUT); // взводим таймер фейдера
        tmFaderTimeout.reset();
        isFaderOn = true;
        faderStep = 1;
      }
      
      #ifdef NEWYEAR_MESSAGE
      NewYearMessagePrint(); // отрабатывает только во включенном состоянии
      #endif
    }

    onOffTimePrint();
    
    //osd_Tick(); // вывод сообщений по методу Palpalych https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proshivki-ot-gunner47.2418/page-26#post-26788

    #ifdef VERTGAUGE
      if(VERTGAUGE==1 && ONflag)
        GaugeShowVertical();
      else if(VERTGAUGE==2 && ONflag)
        GaugeShowHorizontal();
    #endif
    
    if(ONflag || !osd_textDone()){
      FastLED.show();
      #ifdef USELEDBUF
      memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);                             // восстановление кадра с прорисованным эффектом из буфера (без текста и индикаторов) 
      #endif
    }
  }
}

void changePower() // плавное включение/выключение
{
  if (ONflag)
  {
    effectsTick();
    for (uint8_t i = 0U; i < modes[currentMode].Brightness; i = constrain(i + 8, 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    if(lampMode == MODE_DEMO && GlobalBrightness>0)
      FastLED.setBrightness(GlobalBrightness);
    else
      FastLED.setBrightness(modes[currentMode].Brightness);
    delay(2);
    FastLED.show();
  }
  else
  {
    //effectsTick();
    for (uint8_t i = modes[currentMode].Brightness; i > 0; i = constrain(i - 8, 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.clear();
    delay(2);
    FastLED.show();
  }

  #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)          // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
  #endif
  
  TimerManager::TimerRunning = false;
  TimerManager::TimerHasFired = false;
  TimerManager::TimeToFire = 0ULL;

  if (FavoritesManager::UseSavedFavoritesRunning == 0U)     // если выбрана опция Сохранять состояние (вкл/выкл) "избранного", то ни выключение модуля, ни выключение матрицы не сбрасывают текущее состояние (вкл/выкл) "избранного"
  {
      FavoritesManager::TurnFavoritesOff();
  }

  #ifdef USE_MQTT
  if (espMode == 1U)
  {
    MqttManager::needToPublish = true;
  }
  #endif
}
