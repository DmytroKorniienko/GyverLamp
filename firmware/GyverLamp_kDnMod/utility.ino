// служебные функции

// залить все
void fillAll(CRGB color)
{
  for (int32_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

// функция отрисовки точки по координатам X Y
void drawPixelXY(int16_t x, int16_t y, CRGB color)
{
  if (x < 0 || x > (int16_t)(WIDTH - 1) || y < 0 || y > (int16_t)(HEIGHT - 1)) return;
  uint32_t thisPixel = getPixelNumber((uint16_t)x, (uint16_t)y) * SEGMENTS;
  for (uint16_t i = 0; i < SEGMENTS; i++)
  {
    leds[thisPixel + i] = color;
  }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(uint32_t thisSegm)
{
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel > NUM_LEDS - 1) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b);
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t getPixColorXY(uint16_t x, uint16_t y)
{
  return getPixColor(getPixelNumber(x, y));
}

/*
// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif
*/
// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ? (WIDTH - x - 1) : x)
#define THIS_Y (MIRR_H ? (HEIGHT - y - 1) : y)

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X (MIRR_V ? (HEIGHT - y - 1) : y)
#define THIS_Y (MIRR_H ? (WIDTH - x - 1) : x)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ? (WIDTH - x - 1) : x)
#define THIS_Y (MIRR_H ?  x : (WIDTH - x - 1))

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (MIRR_V ? y : (HEIGHT - y - 1))
#define THIS_Y (MIRR_H ? (WIDTH - x - 1) : x)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ?  x : (WIDTH - x - 1))
#define THIS_Y (MIRR_H ? y : (HEIGHT - y - 1))

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (MIRR_V ? y : (HEIGHT - y - 1))
#define THIS_Y (MIRR_H ?  x : (WIDTH - x - 1))

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ?  x : (WIDTH - x - 1))
#define THIS_Y (MIRR_H ? (HEIGHT - y - 1) : y)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X (MIRR_V ? (HEIGHT - y - 1) : y)
#define THIS_Y (MIRR_H ?  x : (WIDTH - x - 1))

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif


// получить номер пикселя в ленте по координатам
uint32_t getPixelNumber(uint16_t x, uint16_t y)
{
  if ((THIS_Y % 2 == 0) || MATRIX_TYPE)                     // если чётная строка
  {
    return ((uint32_t)THIS_Y * _WIDTH + THIS_X);
  }
  else                                                      // если нечётная строка
  {
    return ((uint32_t)THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);
  }
}

#ifdef VERTGAUGE
// настройка индикаторов яркости/скорости/масштаба
byte xStep;
byte xCol;
byte yStep;
byte yCol;
void GaugeSetup()
{
    if(VERTGAUGE){
      xStep = WIDTH / 4;
      xCol = 4;
      if(xStep<2) {
        xStep = WIDTH / 3;
        xCol = 3;
      } else if(xStep<2) {
        xStep = WIDTH / 2;
        xCol = 2;
      } else if(xStep<2) {
        xStep = 1;
        xCol = 1;
      }
    
      yStep = HEIGHT / 4;
      yCol = 4;
      if(yStep<2) {
        yStep = HEIGHT / 3;
        yCol = 3;
      } else if(yStep<2) {
        yStep = HEIGHT / 2;
        yCol = 2;
      } else if(yStep<2) {
        yStep = 1;
        yCol = 1;
      }
    }
}

void GaugeShowVertical() {
  if(!startButtonHolding) return;
  
  switch (numHold) {    // индикатор уровня яркости/скорости/масштаба
    case 1:
      if(currentMode==EFF_WHITE_COLOR)
        ind = sqrt((255.0/(BRIGHTNESS-MIN_WHITE_COLOR_BRGHT))*(modes[currentMode].Brightness-MIN_WHITE_COLOR_BRGHT) + 1); // привести к полной шкале
      else
        ind = sqrt((255.0/BRIGHTNESS)*modes[currentMode].Brightness + 1); // привести к полной шкале
      for (byte x = 0; x <= xCol*(xStep-1) ; x+=xStep) {
        for (byte y = 0; y < HEIGHT ; y++) {
          if (ind > y)
            drawPixelXY(x, y, CHSV(10, 255, 255));
          else
            drawPixelXY(x, y,  0);
        }
      }
      break;
    case 2:
      ind = sqrt(modes[currentMode].Speed - 1);
      for (byte x = 0; x <= xCol*(xStep-1) ; x+=xStep) {
        for (byte y = 0; y <= HEIGHT ; y++) {
          if (ind <= y)
            drawPixelXY(x, HEIGHT-1-y, CHSV(100, 255, 255));
          else
            drawPixelXY(x, HEIGHT-1-y,  0);
        }
      }
      break;
    case 3:
      ind = sqrt(modes[currentMode].Scale + 1);
      for (byte x = 0; x <= xCol*(xStep-1) ; x+=xStep) {
        for (byte y = 0; y < HEIGHT ; y++) {
          if (ind > y)
            drawPixelXY(x, y, CHSV(150, 255, 255));
          else
            drawPixelXY(x, y,  0);
        }
      }
      break;
  }
}

void GaugeShowHorizontal() {
  if(!startButtonHolding) return;
  
  switch (numHold) {    // индикатор уровня яркости/скорости/масштаба
    case 1:
      if(currentMode==EFF_WHITE_COLOR)
        ind = sqrt((255.0/(BRIGHTNESS-MIN_WHITE_COLOR_BRGHT))*(modes[currentMode].Brightness-MIN_WHITE_COLOR_BRGHT) + 1); // привести к полной шкале
      else
        ind = sqrt((255.0/BRIGHTNESS)*modes[currentMode].Brightness + 1); // привести к полной шкале
      for (byte y = 0; y <= yCol*(yStep-1) ; y+=yStep) {
        for (byte x = 0; x < WIDTH ; x++) {
          if (ind > x)
            drawPixelXY((x+y)%WIDTH, y, CHSV(10, 255, 255));
          else
            drawPixelXY((x+y)%WIDTH, y,  0);
        }
      }
      break;
    case 2:
      ind = sqrt(modes[currentMode].Speed - 1);
      for (byte y = 0; y <= yCol*(yStep-1) ; y+=yStep) {
        for (byte x = 0; x <= WIDTH ; x++) {
          if (ind < x)
            drawPixelXY((WIDTH-x+y)%WIDTH, y, CHSV(100, 255, 255));
          else
            drawPixelXY((WIDTH-x+y)%WIDTH, y,  0);
        }
      }
      break;
    case 3:
      ind = sqrt(modes[currentMode].Scale + 1);
      for (byte y = 0; y <= yCol*(yStep-1) ; y+=yStep) {
        for (byte x = 0; x < WIDTH ; x++) {
          if (ind > x)
            drawPixelXY((x+y)%WIDTH, y, CHSV(150, 255, 255));
          else
            drawPixelXY((x+y)%WIDTH, y,  0);
        }
      }
      break;
  }
}
#endif
