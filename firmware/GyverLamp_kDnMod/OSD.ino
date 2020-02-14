// OSD v1.0b / On Screen Display -- работа с бегущим текстом и графическими представлениями для матриц на ws821х для библиотеки FastLED
// TODO: переместить функции прорисовки индикаторов

char osd_string[32];                                                        // буфер бегущей строки макс. 32 символа [0...31], [32] = 00 конец строки
uint32_t osd_scrollTimer = 0LL;                                             // таймер бегущей строки
int16_t osd_offset = 0;                                                     // переменная текущего столбца матрицы
CRGB osd_fontColor = CRGB::White;                                           // переменная цвета текста
                                                                            // TODO: <- перенести константы для работы с текстом из других модулей (они не относятся к конфигурации лампы)

const int16_t let_width = LET_WIDTH - 1 ;                                   // константы
const int16_t let_space = LET_WIDTH + SPACE;                                //
const int16_t width = WIDTH - 1;                                            //

bool osd_Tick() {                                                           // тикер вывода текста
  if (!osd_string[0]) return false;                                         // текст полностью был показан - выход из функции
  if (millis() - osd_scrollTimer >= 100) {                                  // наступил момент смещения текста на 1 пиксель
    osd_scrollTimer = millis(); osd_offset++;
  }

  if(ONflag){
    for (uint16_t i = 0; i < WIDTH; i++) {                                  // цикл прорисовки темного фона под бегущую ленту
      for (uint16_t j = 1; j < 10; j++) {                                   // TODO: по идее можно сделать эффекты фейдера ленты до и после прокрутки текста
        leds[getPixelNumber(i, j)] %= 16;                                   // TODO: эффекты фейдера можно сделать различными
      }
    }
  }

  if (osd_fillString()) {                                                   // прорисовка текста на экране
    osd_offset = 0; osd_string[0] = 0; return false;                        // текст полностью вывелся на экран, сброс папаметров, процедура готова к получению нового текста.
  }
}

bool osd_textDone() {
  return !osd_string[0] ;                                                   // возвращает признак завершения показа строки
}

bool osd_printText(String text, CRGB color) {                               // печать бегущей строки
  if (text == "" || osd_string[0]) return false;                            // процедура не приняла значение или находится на стадии печати строки (возвращает false)
  osd_fontColor = color;                                                    // сохранение текущего цвета
  text.toCharArray(osd_string, text.length() + 1);                          // сохранение печатоемой строки в буфере
  return true;                                                              // показ строки начался (возвращает true) 
}

bool osd_printCurrentTime(CRGB color) {                                     // печать текущего времени // TODO: переписать поцедуру для вывода правильного формата "->00:00"
  if (osd_string[0]) return false;                                          // процедура находится на стадии печати строки
#if defined(USE_NTP) && defined(PRINT_TIME)                                 // вывод, только если используется синхронизация времени и если заказан его вывод бегущей строкой
  if (isWifiOffMode) return false;                                          // при отключенном WiFi - сразу на выход
  if (espMode != 1U || !ntpServerAddressResolved || !timeSynched) {         // вывод только в режиме WiFi клиента и только, если имя сервера времени разрезолвлено
    return osd_printText("WiFi OFF", CRGB::Red);
  }
  return osd_printText(" ->" + String((uint8_t)((thisTime - thisTime % 60U) / 60U)) + ":" + String((uint8_t)(thisTime % 60U))+ " ", color);
#endif
  return false;                                                             //без интернета возвращаем
}

bool osd_fillString() {                                                     // прорисовка части строки относительно заданного смещения / TODO: проверить работу с русскими буквами
  int16_t len = strlen(osd_string);
  if (osd_offset > let_space * len + width) return true;                    // если вся строка напечатана возвращаем признак
  for  (int16_t i = 0; i < len; i++) {                                      // перечисление строки по символам
    uint16_t loffset = osd_offset - i * (let_space);
    osd_fillLetter(osd_string[i], loffset);
  }
  return false;                                                             // строка еще печатается
}

void osd_fillLetter(uint16_t letter, int16_t offset) {                      // прорисовка заданного символа относительно его положения на экране
  if (offset < 0  || offset - let_width > width ) return;                   // если символ целиком находится вне матрицы - выход их процедуры
  for (int16_t i = 0; i <= let_width; i++) {                                // прорисовка столбцов единичного символа
    if (offset - i >= 0 && offset - i <=  width) {                          // исполняем, если столбец находится в матрице
      uint8_t thisByte = getFont(letter, (uint8_t)(i));                     // вычисление столбца
      for (int16_t j = 0; j < LET_HEIGHT; j++) {                            // цикл побитового вывода
        bool thisBit =  thisByte & (1 << (LET_HEIGHT - 1 - j)) ;            // вычисление пикселя
        if (thisBit) leds[getPixelNumber((uint16_t)(width - offset + i), (uint16_t)(TEXT_HEIGHT + j))] = osd_fontColor; // печатаем пиксель заданным цветом, если необходимо
      }
    }
  }
}
