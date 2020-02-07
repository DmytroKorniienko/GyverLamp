#pragma once

class TimerManager
{
  public:
    static bool TimerRunning;                               // флаг "таймер взведён"
    static bool TimerHasFired;                              // флаг "таймер отработал"
    static uint8_t TimerOption;                             // индекс элемента в списке List Picker'а
    static uint64_t TimeToFire;                             // время, в которое должен сработать таймер (millis)

    static void HandleTimer(                                // функция, обрабатывающая срабатывание таймера, гасит матрицу
      bool* ONflag,
      bool* settChanged,
      uint32_t* eepromTimeout,
      void (*changePower)())
    {
      if (!TimerManager::TimerHasFired &&
           TimerManager::TimerRunning &&
           millis() >= TimerManager::TimeToFire)
      {
        #ifdef GENERAL_DEBUG
        LOG.print(F("Выключение по таймеру\n\n"));
        #endif

        TimerManager::TimerRunning = false;
        TimerManager::TimerHasFired = true;
        FastLED.clear();
        delay(2);
        FastLED.show();
        *ONflag = !(*ONflag);
        changePower();
        *settChanged = true;
        *eepromTimeout = millis();
      }
    }
};

//----------------------------------------------------------------

// мини-класс таймера, версия 1.0

class timerMinim
{
  public:
    timerMinim(uint32_t interval);				                  // объявление таймера с указанием интервала
    void setInterval(uint32_t interval);	                  // установка интервала работы таймера
    bool isReady();						                              // возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
    bool isReadyManual();                                   // возвращает true, когда пришло время. Без сбороса
    void reset();							                              // ручной сброс таймера на установленный интервал

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
};

timerMinim::timerMinim(uint32_t interval)
{
  _interval = interval;
  _timer = millis();
}

void timerMinim::setInterval(uint32_t interval)
{
  _interval = interval;
}

bool timerMinim::isReady()
{
  if ((uint32_t)millis() - _timer >= _interval)
  {
    _timer = millis();
    return true;
  }
  else
  {
    return false;
  }
}

bool timerMinim::isReadyManual()
{
  if ((uint32_t)millis() - _timer >= _interval)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void timerMinim::reset()
{
  _timer = millis();
}
