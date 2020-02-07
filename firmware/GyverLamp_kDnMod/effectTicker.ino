uint32_t effTimer;
byte ind;

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
      if(millis() - effTimer >= modes[currentMode].Speed){
        EFFECTS_ARR[currentMode].func(EFFECTS_ARR[currentMode].param);
        effTimer = millis();
      }

      #ifdef NEWYEAR_MESSAGE
      NewYearMessagePrint(); // отрабатывает только во включенном состоянии
      #endif
      
      #ifdef VERTGAUGE
        if(VERTGAUGE==1)
          GaugeShowVertical();
        else if(VERTGAUGE==2)
          GaugeShowHorizontal();
      #endif
    }

    onOffTimePrint();
    
    if(ONflag)
      FastLED.show();
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
