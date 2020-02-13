// работа с бегущим текстом
int32_t offset = (MIRR_V ? 0 : WIDTH);
uint32_t scrollTimer = 0LL;

bool fillStringManual(const char* text, CRGB letterColor, bool stopText)
{
  if (!text || !strlen(text))
  {
    return true;
  }

  uint16_t i = 0, j = 0;
  while (text[i] != '\0')
  {
    if ((uint8_t)text[i] > 191)                           // работаем с русскими буквами
    {
      i++;
    }
    else
    {
      if(!MIRR_V)
        drawLetter(text[i], offset + (int16_t)j * (LET_WIDTH + SPACE), letterColor);
      else
        drawLetter(text[i], offset - (int16_t)j * (LET_WIDTH + SPACE), letterColor);
      i++;
      j++;
    }
  }

  if(!stopText)
    (MIRR_V ? offset++ : offset--);
  if ((!MIRR_V && offset < (int32_t)(-j * (LET_WIDTH + SPACE))) || (MIRR_V && offset > (int32_t)(j * (LET_WIDTH + SPACE))+WIDTH))       // строка убежала
  {
    offset = (MIRR_V ? 0 : WIDTH);
    return true;
  }

  return false;
}

bool fillString(const char* text, CRGB letterColor)
{
  if (!text || !strlen(text))
  {
    return true;
  }

  if (millis() - scrollTimer >= TEXT_SCROLL_SPEED)
  {
    scrollTimer = millis();
    //FastLED.clear();
    uint16_t i = 0, j = 0;
    while (text[i] != '\0')
    {
      if ((uint8_t)text[i] > 191)                           // работаем с русскими буквами
      {
        i++;
      }
      else
      {
        if(!MIRR_V)
          drawLetter(text[i], offset + (int16_t)j * (LET_WIDTH + SPACE), letterColor);
        else
          drawLetter(text[i], offset - (int16_t)j * (LET_WIDTH + SPACE), letterColor);
        i++;
        j++;
      }
    }

    (MIRR_V ? offset++ : offset--);
    if ((!MIRR_V && offset < (int32_t)(-j * (LET_WIDTH + SPACE))) || (MIRR_V && offset > (int32_t)(j * (LET_WIDTH + SPACE))+WIDTH))       // строка убежала
    {
      offset = (MIRR_V ? 0 : WIDTH);
      return true;
    }

    //if(!ONflag) // только для случая вывода времени при выключенном состоянии
      FastLED.show();    
  }

  return false;
}

bool printTime(uint32_t thisTime, bool onDemand, bool ONflag, bool isManual, bool isStopped) // периодический вывод времени бегущей строкой; onDemand - по требованию, вывод текущего времени; иначе - вывод времени по расписанию; isManual == true (ручной режим)
{
  #ifdef USE_NTP                                            // вывод, только если используется синхронизация времени

  //if(isWifiOffMode) return false; // при отключенном WiFi - сразу на выход

  //if ((espMode != 1U || !ntpServerAddressResolved || !timeSynched) && (!isWifiOffMode))     // вывод только в режиме WiFi клиента и только, если имя сервера времени разрезолвлено
  //{
  //  showWarning(CRGB::Red, 4000U, 500U);                    // мигание красным цветом 4 секунды
  //  return false;
  //}

  CRGB letterColor = CRGB::Black;
  bool needToPrint = false;
  
  #if (PRINT_TIME >= 1U)                                    // вывод только каждый час (красным цветом)
  if (thisTime % 60U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Red;
  }
  #endif

  #if (PRINT_TIME == 2U)                                    // вывод каждый час (красным цветом) + каждые 30 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 30U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 3U)                                    // вывод каждый час (красным цветом) + каждые 15 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 15U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 4U)                                    // вывод каждый час (красным цветом) + каждые 10 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 10U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 5U)                                    // вывод каждый час (красным цветом) + каждые 5 минут (синим цветом)
  if (thisTime % 60U != 0U && thisTime % 5U == 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  #if (PRINT_TIME == 6U)                                    // вывод каждый час (красным цветом) + каждую минуту (синим цветом)
  if (thisTime % 60U != 0U)
  {
    needToPrint = true;
    letterColor = CRGB::Blue;
  }
  #endif

  if (onDemand)
  {
    if(lampMode!=MODE_ALARMCLOCK)
      letterColor = CRGB::White;
    else
      hsv2rgb_rainbow(dawnColor, letterColor); // конвертация цвета времени, с учетом текущей точки рассвета
  }

  if ((needToPrint && thisTime != lastTimePrinted) || onDemand)
  {
    char stringTime[15U];                                   // буффер для выводимого текста, его длина должна быть НЕ МЕНЬШЕ, чем длина текста + 1
    if(!ntpServerAddressResolved || !timeSynched)
      sprintf_P(stringTime, PSTR("!syn -> %u:%02u"), (uint8_t)((thisTime - thisTime % 60U) / 60U), (uint8_t)(thisTime % 60U));
    else
      sprintf_P(stringTime, PSTR("-> %u:%02u"), (uint8_t)((thisTime - thisTime % 60U) / 60U), (uint8_t)(thisTime % 60U));    

    if(lampMode == MODE_DEMO && GlobalBrightness>0)
      FastLED.setBrightness(GlobalBrightness);
    else if(lampMode != MODE_ALARMCLOCK)
      FastLED.setBrightness(getBrightnessForPrintTime(thisTime, ONflag));
    delay(1);

    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)        // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
    #endif

    if(!isManual)
      while (!fillString(stringTime, letterColor)) {
      #ifdef ESP_USE_BUTTON
      if (buttonEnabled){
        buttonTick();
      }
      #endif
        delay(1);
        ESP.wdtFeed();
        }
    else
       if(!fillStringManual(stringTime, letterColor, isStopped)){
         return false;
       }
    
    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)        // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
    digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
    #endif

    lastTimePrinted = thisTime;
      
    return true;
  }
  return false;
  #endif
}


uint8_t getBrightnessForPrintTime(uint32_t thisTime, bool ONflag)     // определение яркости для вывода времени бегущей строкой в зависимости от ESP_MODE, USE_NTP, успешности синхронизации времени,
                                                                      // текущего времени суток, настроек дневного/ночного времени и того, включена ли сейчас матрица
{
  #if defined(USE_NTP) && defined(PRINT_TIME)

  if (espMode != 1U || !ntpServerAddressResolved || ONflag)
  {
    return modes[currentMode].Brightness;
  }

  if (NIGHT_HOURS_START >= NIGHT_HOURS_STOP)                          // ночное время включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START || thisTime <= NIGHT_HOURS_STOP)// период действия ночного времени
    {
      return (NIGHT_HOURS_BRIGHTNESS >= 0)
        ? NIGHT_HOURS_BRIGHTNESS
        : modes[currentMode].Brightness;
    }
  }
  else                                                                // ночное время не включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START && thisTime <= NIGHT_HOURS_STOP)// период действия ночного времени
    {
      return (NIGHT_HOURS_BRIGHTNESS >= 0)
        ? NIGHT_HOURS_BRIGHTNESS
        : modes[currentMode].Brightness;
    }
  }

  return (DAY_HOURS_BRIGHTNESS >= 0)                                  // дневное время
    ? DAY_HOURS_BRIGHTNESS
    : modes[currentMode].Brightness;

  #endif

  return modes[currentMode].Brightness;
}


void drawLetter(uint16_t letter, int16_t offset, CRGB letterColor)
{
  uint16_t start_pos = 0, finish_pos = LET_WIDTH + SPACE;

  if (offset < (int16_t)-LET_WIDTH || offset > (int16_t)WIDTH)
  {
    return;
  }
  if (offset < 0)
  {
    start_pos = (uint16_t)-offset;
  }
  if (offset > (int16_t)(WIDTH - LET_WIDTH))
  {
    finish_pos = (uint16_t)(WIDTH - offset);
  }

  for (uint16_t i = start_pos; i < finish_pos; i++)
  {
    uint8_t thisByte;

    if((finish_pos - i <= SPACE) || ((LET_WIDTH - 1 - i)<0))
      thisByte = 0x00;
    else if (MIRR_V)
    {
      thisByte = getFont(letter, (uint16_t)(LET_WIDTH - 1 - i));
    }
    else
    {
      thisByte = getFont(letter, i);
    }

    //if(textinverse) thisByte=~thisByte;

    for (uint16_t j = 0; j < LET_HEIGHT; j++)
    {
      bool thisBit = MIRR_H
        ? thisByte & (1 << j)
        : thisByte & (1 << (LET_HEIGHT - 1 - j));

      if(TEXT_DIRECTION){
        if(MIRR_V){
          drawPixelXY(offset - 1, TEXT_HEIGHT + j, (textinverse ? letterColor : CRGB::Black)); // очистить предыдущий
        }
      } else {
        if(MIRR_V){
          drawPixelXY(j + TEXT_HEIGHT, offset - 1, (textinverse ? letterColor : CRGB::Black)); // очистить предыдущий
        }
      }
      
      // рисуем столбец (i - горизонтальная позиция, j - вертикальная)
      if (TEXT_DIRECTION)
      {
        if (thisBit)
        {
          drawPixelXY(offset + i, TEXT_HEIGHT + j, (!textinverse ? letterColor : CRGB::Black));
        }
        else
        {
          drawPixelXY(offset + i, TEXT_HEIGHT + j, (textinverse ? letterColor : CRGB::Black));
        }
      }
      else
      {
        if (thisBit)
        {
          drawPixelXY(j + TEXT_HEIGHT, offset + i, (!textinverse ? letterColor : CRGB::Black));
        }
        else
        {
          drawPixelXY(j + TEXT_HEIGHT, offset + i, (textinverse ? letterColor : CRGB::Black));
        }
      }
    }
  }
}


// --- СЛУЖЕБНЫЕ ФУНКЦИИ ---------------
uint8_t getFont(uint8_t asciiCode, uint8_t row)             // интерпретатор кода символа в массиве fontHEX (для Arduino IDE 1.8.* и выше)
{
  asciiCode = asciiCode - '0' + 16;                         // перевод код символа из таблицы ASCII в номер согласно нумерации массива

  if (asciiCode <= 90)                                      // печатаемые символы и английские буквы
  {
    return pgm_read_byte(&fontHEX[asciiCode][row]);
  }
  else if (asciiCode >= 112 && asciiCode <= 159)
  {
    return pgm_read_byte(&fontHEX[asciiCode - 17][row]);
  }
  else if (asciiCode >= 96 && asciiCode <= 111)
  {
    return pgm_read_byte(&fontHEX[asciiCode + 47][row]);
  }

  return 0;
}
