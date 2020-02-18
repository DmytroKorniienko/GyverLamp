// ************* НАСТРОЙКИ *************
// ************* ДЛЯ РАЗРАБОТЧИКОВ *****
// The 16 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

uint16_t speed = 20;                                        // speed is set dynamically once we've started up
uint16_t scale = 30;                                        // scale is set dynamically once we've started up

// This is the array that we keep our computed noise values in
#define MAX_DIMENSION (max(WIDTH, HEIGHT))
#if (WIDTH > HEIGHT)
uint8_t noise[WIDTH][WIDTH];
#else
uint8_t noise[HEIGHT][HEIGHT];
#endif

CRGBPalette16 currentPalette(PartyColors_p);
uint8_t colorLoop = 1;
uint8_t ihue = 0;

// ============= ЭФФЕКТЫ ===============
// ------------- конфетти --------------
void sparklesRoutine(char *param)
{
  for (uint8_t i = 0; i < modes[EFF_SPARKLES].Scale; i++)
  {
    uint8_t x = random(0U, WIDTH);
    uint8_t y = random(0U, HEIGHT);
    if (getPixColorXY(x, y) == 0U)
    {
      leds[getPixelNumber(x, y)] = CHSV(random(0U, 255U), 255U, 255U);
    }
  }
  fader(FADE_OUT_SPEED);
}

// функция плавного угасания цвета для всех пикселей
void fader(uint8_t step)
{
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      fadePixel(i, j, step);
    }
  }
}

void fadePixel(uint8_t i, uint8_t j, uint8_t step)          // новый фейдер
{
  int32_t pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0U) return;

  if (leds[pixelNum].r >= 30U ||
      leds[pixelNum].g >= 30U ||
      leds[pixelNum].b >= 30U)
  {
    leds[pixelNum].fadeToBlackBy(step);
  }
  else
  {
    leds[pixelNum] = 0U;
  }
}

// ------------- огонь -----------------

// ----- неперемещаемые/неизменные оперделения -----
#define FIRE_FLAMEWIDTH           (8U)       // ширина очага (маски)
#define FIRE_FLAMEHEIGHT          (8U)       // высота очага (маски)
const uint8_t _bedshiftmin = FIRE_FLAMEWIDTH - FIRE_CORESHIFT ;
const uint8_t _bedshiftmax = FIRE_FLAMEWIDTH + FIRE_CORESHIFT ;
const uint8_t _flameshiftmin = FIRE_FLAMEWIDTH - FIRE_FLAMESHIFT ;
const uint8_t _flameshiftmax = FIRE_FLAMEWIDTH + FIRE_FLAMESHIFT ;
const uint8_t _corestrength = FIRE_CORESTRENGTH + 1U ;
const uint8_t _coreactivity = 255U - FIRE_COREACTIVITY ;
const uint8_t _corespeed = FIRE_CORESPEED + 1U ;
const uint8_t _flamestrength = FIRE_FLAMESTRENGTH + 1U ;
const uint8_t _flamespeed = FIRE_FLAMESPEED + 1U ;
const uint8_t _sparkstrength = FIRE_SPARKSTRENGTH + 1U ;

uint8_t matrixValue[WIDTH][FIRE_FLAMEHEIGHT];
uint8_t line[WIDTH];
uint8_t pcnt = 0U;
uint8_t deltaHue = FIRE_FLAMEWIDTH;                    // текущее смещение пламени (hueMask)
uint8_t shiftHue[HEIGHT];                              // массив дороожки горизонтального смещения пламени (hueMask)
uint8_t deltaValue = FIRE_FLAMEWIDTH;                  // текущее смещение пламени (hueValue)
uint8_t shiftValue[HEIGHT];                            // массив дороожки горизонтального смещения пламени (hueValue)
uint8_t rowcounter;

//these values are substracetd from the generated values to give a shape to the animation
static const uint8_t valueMask[FIRE_FLAMEHEIGHT][FIRE_FLAMEWIDTH] PROGMEM =
{
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
static const uint8_t hueMask[FIRE_FLAMEHEIGHT][FIRE_FLAMEWIDTH] PROGMEM =
{
  {1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};
void fireRoutine(char *isColored)                            // C - цветной огонь, остальное - белый
//void fireRoutine(bool isColored)                           // ******* для оригинальной прошивки Gunner47 *******
{
  /*if (loadingFlag) {
    loadingFlag = false;
    //FastLED.clear();
    generateLine();
    //memset(matrixValue, 0, sizeof(matrixValue)); без очистки
    }*/
  if (pcnt >= FIRE_PLUMEACTIVITY) {                         // внутренний делитель кадров для поднимающегося пламени
    generateLine();                                         // перерисовать новую нижнюю линию случайным образом
    pcnt %= FIRE_PLUMEACTIVITY;
  }
  shiftUp();                                                // смещение кадра вверх
  drawFrame(FIRE_PLUMEACTIVITY - pcnt, (strcmp(isColored, "C") == 0));            // прорисовка экрана
  //drawFrame(pcnt, isColored);                                                   // ******* прорисовка экрана для оригинальной прошивки Gunner47 *******
  pcnt += FIRE_PLUMEFREQUENCY;                              // делитель кадров: задает скорость подъема пламени 25/100 ~ 1/4
}

// Randomly generate the next line (matrix row)
void generateLine() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    line[x] = random(_coreactivity, 256);                   // заполнение случайным образом нижней линии теневой маски FIRE_COREACTIVITY =255 - без контраста, =192 - оригинал
  }
}

void shiftUp() {                                            //подъем кадра теневой маски
  for (uint8_t y = FIRE_FLAMEHEIGHT - 1U; y > 0U; y--) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      matrixValue[x][y] = matrixValue[x][y - 1U];           //смещение пламени теневой маски (только для зоны очага)
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++) {                                      // прорисовка новой нижней линии теневой маски
    matrixValue[x][0U] = line[x];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation
void drawFrame(uint8_t pcnt, bool isColored) {                                // прорисовка нового кадра
  int32_t nextv;
#ifdef FIRE_UNIVERSE                                                          // если определен универсальный огонь вычисляем его базовый цвет  
  uint8_t baseHue = (float)modes[EFF_FIRE].Scale * 2.57;
  uint8_t baseSat = (modes[EFF_FIRE].Scale == FIRE_WHITEVALUE) ? 0U : 255U ;  // отключение насыщенности при определенном положении колеса
#else
  uint8_t baseHue = isColored ? 255U : 0U;                                    // старое определение цвета
  uint8_t baseSat = 255;
#endif

  //first row interpolates with the "next" line
#if FIRE_CORESTRENGTH                                                         // смещение очага 
  deltaValue = random(0U, _corestrength) && !(rowcounter % FIRE_CORERESIST) ? constrain (shiftValue[0] + random(0U, _corespeed) - random(0U, _corespeed),
               _bedshiftmin, _bedshiftmax) : shiftValue[0];
  shiftValue[0] = deltaValue;
#endif

#if FIRE_FLAMESTRENGTH                                                        // смещение от колыхания пламени
  deltaHue = random(0U, _flamestrength) && !(rowcounter % FIRE_FLAMERESIST) ? constrain (shiftHue[0] + random(0U, _flamespeed) - random(0U, _flamespeed),
             _flameshiftmin, _flameshiftmax) : shiftHue[0];
  shiftHue[0] = deltaHue;                                                     // заносим это значение в стэк
#endif
  rowcounter++;

  for (uint8_t x = 0U; x < WIDTH; x++) {                                      // прорисовка нижней строки (сначала делаем ее, так как потом будем пользоваться ее значением смещения)
    nextv =                                                                   // расчет значения яркости относительно valueMask и нижерасположенной строки.
      (( pcnt * matrixValue[x][0] + (255.0 - pcnt) * line[x]) / 255.0) + random(0, FIRE_VOLUMESEED)
      - pgm_read_byte(&valueMask[0][(x + FIRE_MASKOFFSET + deltaValue) % FIRE_FLAMEWIDTH]);
    CRGB color = CHSV(                                                        // вычисление цвета и яркости пикселя
                   baseHue + random(0, FIRE_HUESEED) + pgm_read_byte(&hueMask[0][(x + FIRE_MASKOFFSET + deltaValue + deltaHue) % FIRE_FLAMEWIDTH]),      // H - смещение всполохов
                   baseSat,                                                   // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                   (uint8_t)constrain(nextv, 0, 255));                        // V
    leds[getPixelNumber(x, 0)] = color;                                       // прорисовка цвета очага
  }

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--) {                                // прорисовка остальных строк с учетом значения низлежащих
    deltaValue = shiftValue[y];                                               // извлекаем положение яркости частицы
    shiftValue[y] = shiftValue[y - 1];                                        // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем
    deltaHue = shiftHue[y];                                                   // извлекаем положение оттенка частицы
    shiftHue[y] = shiftHue[y - 1];                                            // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем

    if (y > FIRE_FLAMEHEIGHT) {                                               // цикл стирания текущей строоки для искр
      for (uint8_t _x = 0U; _x < WIDTH; _x++) {                               // стираем строчку с искрами (очень не оптимально)
        drawPixelXY(_x, y, 0U);
      }
    }
    for (uint8_t x = 0U; x < WIDTH; x++) {                                    // пересчет координаты x для текущей строки
      if (y < 8U) {                                                           // если строка представляет очаг
        nextv =                                                               // расчет значения яркости относительно valueMask и нижерасположенной строки.
          (( pcnt * matrixValue[x][y]
             + (255.0 - pcnt) * matrixValue[x][y - 1]) / 255.0) + random(0, FIRE_VOLUMESEED)
          - pgm_read_byte(&valueMask[y][(x + FIRE_MASKOFFSET + deltaValue) % FIRE_FLAMEWIDTH]);

        CRGB color = CHSV(                                                    // определение цвета пикселя
                       baseHue + random(0, FIRE_HUESEED) + pgm_read_byte(&hueMask[y][(x + FIRE_MASKOFFSET + deltaValue + deltaHue) % FIRE_FLAMEWIDTH ]),         // H - смещение всполохов
                       baseSat,                                               // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                       (uint8_t)constrain(nextv, 0, 255));                    // V

        leds[getPixelNumber(x, y)] = color;
      }
      else if (y == FIRE_FLAMEHEIGHT && FIRE_SPARKLES) {                      // если это самая нижняя строка искр - формитуем искорку из пламени
        if (random(0, FIRE_EVERYNSPARK) == 0 && getPixColorXY(x, y - 1U) != 0U) {
          uint16_t px = getPixelNumber (x, y);
          leds[px] = getPixColorXY(x, y - 2U);
          leds[px] %= FIRE_SPARKSBRIGHT;
        } else drawPixelXY(x, y, 0U);
      }
      else if (FIRE_SPARKLES) {                                               // если это не самая нижняя строка искр - перемещаем искорку выше
#if FIRE_SPARKSTRENGTH
        uint8_t newX = (random(0, _sparkstrength)) ? x : (x + WIDTH + random(0U, 2U) - random(0U, 2U)) % WIDTH ;   // с вероятностью 1/3 смещаем искорку влево или вправо
#else
        uint8_t newX = x;
#endif
        if (getPixColorXY(x, y - 1U) > 0U) drawPixelXY(newX, y, getPixColorXY(x, y - 1U));    // рисуем искорку на новой строчке
      }
    }
  }
}

// ============= водо/огне/лава/радуга/хренопад ===============
void fire2012WithPalette(char *isColored) {
  //bool fire_water = modes[EFF_WATERFALL].Scale <= 50;
  //uint8_t COOLINGNEW = fire_water ? modes[EFF_WATERFALL].Scale * 2  + 20 : (100 - modes[EFF_WATERFALL].Scale ) *  2 + 20 ;
  uint8_t scale = modes[EFF_WATERFALL].Scale;
  uint8_t COOLINGNEW = constrain((uint16_t)(scale % 16) * 32 / HEIGHT + 16, 1, 255) ;
  // Array of temperature readings at each simulation cell
  static byte heat[WIDTH][HEIGHT];

  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Cool down every cell a little
    for (int i = 0; i < HEIGHT; i++) {
      //heat[x][i] = qsub8(heat[x][i], random8(0, ((COOLINGNEW * 10) / HEIGHT) + 2));
      heat[x][i] = qsub8(heat[x][i], random8(0, COOLINGNEW));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = HEIGHT - 1; k >= 2; k--) {
      heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKINGNEW) {
      int y = random8(2);
      heat[x][y] = qadd8(heat[x][y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < HEIGHT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8(heat[x][j], 240);
      if  (scale < 16) {            // Lavafall
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(LavaColors_p, colorindex);
      } else if (scale < 32) {      // Firefall
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(HeatColors_p, colorindex);
      } else if (scale < 48) {      // Waterfall
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(WaterfallColors_p, colorindex);
      } else if (scale < 64) {      // Skyfall
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(CloudColors_p, colorindex);
      } else if (scale < 80) {      // Forestfall
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(ForestColors_p, colorindex);
      } else if (scale < 96) {      // Rainbowfall
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(RainbowColors_p, colorindex);        
      } else {                      // Aurora
        leds[getPixelNumber(x, (HEIGHT - 1) - j)] = ColorFromPalette(RainbowStripeColors_p, colorindex);

      }
    }
  }
}

// ------------- радуга вертикальная ----------------
uint8_t hue;
void rainbowVerticalRoutine(char *param)
{
  hue += 4;
  for (uint8_t j = 0; j < HEIGHT; j++)
  {
    CHSV thisColor = CHSV((uint8_t)(hue + j * modes[EFF_RAINBOW_VER].Scale), 255, 255);
    for (uint8_t i = 0U; i < WIDTH; i++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга горизонтальная ----------------
void rainbowHorizontalRoutine(char *param)
{
  hue += 4;
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    CHSV thisColor = CHSV((uint8_t)(hue + i * modes[EFF_RAINBOW_HOR].Scale), 255, 255);
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга диагональная -------------
void rainbowDiagonalRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }

  hue += 8;
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      float twirlFactor = 3.0F * (modes[EFF_RAINBOW_DIAG].Scale / 100.0F);      // на сколько оборотов будет закручена матрица, [0..3]
      CRGB thisColor = CHSV((uint8_t)(hue + ((float)WIDTH / (float)HEIGHT * i + j * twirlFactor) * ((float)255 / (float)maxDim)), 255, 255);
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- цвета -----------------
void colorsRoutine(char *param)
{
  //if (loadingFlag)
  {
    hue += modes[EFF_COLORS].Scale;

    for (uint16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(hue, 255U, 255U);
    }
  }
}

// ------------- цвет ------------------
void colorRoutine(char *param)
{
  //if (loadingFlag)
  {
    loadingFlag = false;
    //FastLED.clear();

    for (int16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(modes[EFF_COLOR].Scale * 2.5, 255U, 255U);
    }
  }
}

// ------------- снегопад ----------
void snowRoutine(char *param)
{
  // сдвигаем всё вниз
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    for (uint8_t y = 0U; y < HEIGHT - 1; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1U));
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2U) == 0U && (random(0, 100 - modes[EFF_SNOW].Scale) == 0U))
      drawPixelXY(x, HEIGHT - 1U, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1U, 0x000000);
  }
}

// ------------- метель -------------
void snowStormRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  
  // заполняем головами комет левую и верхнюю линию
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    if (getPixColorXY(0U, i) == 0U &&
       (random(0, SNOW_DENSE) == 0) &&
        getPixColorXY(0U, i + 1U) == 0U &&
        getPixColorXY(0U, i - 1U) == 0U)
    {
      leds[getPixelNumber(0U, i)] = CHSV(random(0, 200), SNOW_SATURATION, 255U);
    }
  }
  
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, map(modes[EFF_SNOWSTORM].Scale, 0U, 255U, 10U, 120U)) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(random(0, 200), SNOW_SATURATION, 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    fadePixel(0U, i, SNOW_TAIL_STEP);
  }
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    fadePixel(i, HEIGHT - 1U, SNOW_TAIL_STEP);
  }
}

// ------------- звездопад -------------
void starfallRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  
  // заполняем головами комет левую и верхнюю линию
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    if (getPixColorXY(0U, i) == 0U &&
       (random(0, STAR_DENSE) == 0) &&
        getPixColorXY(0U, i + 1U) == 0U &&
        getPixColorXY(0U, i - 1U) == 0U)
    {
      leds[getPixelNumber(0U, i)] = CHSV(random(0, 200), STAR_SATURATION, 255U);
    }
  }
  
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, map(modes[EFF_STARFALL].Scale, 0U, 255U, 10U, 120U)) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(random(0, 200), STAR_SATURATION, 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    fadePixel(0U, i, STAR_TAIL_STEP);
  }
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    fadePixel(i, HEIGHT - 1U, STAR_TAIL_STEP);
  }
}

// ------------- матрица ---------------
void matrixRoutine(char *param) // Вариация от Сотнег
{
  // обрабатываем верхний ряд пикселей матрицы
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // ох, если бы я сразу знал, что getPixColorXY возвращает другое значение, не то, которое было отправлено через drawPixelXY
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1U);
    if (thisColor == 0U)                                                                                          // если верхний пиксель не горит,
      drawPixelXY(x, HEIGHT - 1U, (0x99ff00) * (random(0, 100 - modes[EFF_MATRIX].Scale) == 0U));                 //   заполняем его с вероятностью .Scale  + признак скорости
    else if (thisColor <= 0x0d1406)                                                                               // если же он почти потух,
      drawPixelXY(x, HEIGHT - 1U, 0U);                                                                            //   гасим его окончательно
    else if (thisColor >= 0x96fc00)                                                                               // если он максимальной яркости,
      drawPixelXY(x, HEIGHT - 1U, thisColor - 0x467700);  //284400                                                //   резко снижаем яркость
    else                                                                                                          // а иначе,
      drawPixelXY(x, HEIGHT - 1U, thisColor - 0x0a1100);                                                          //   снижаем яркость на 1 уровень
  }
    
  // сдвигаем всё вниз
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1U));                                                                //   просто копируем пиксель на пиксель ниже него)
    }
  }
}
// ------------- светлячки --------------
int32_t lightersPos[2U][LIGHTERS_AM];
int8_t lightersSpeed[2U][LIGHTERS_AM];
CHSV lightersColor[LIGHTERS_AM];
uint8_t loopCounter;
int32_t angle[LIGHTERS_AM];
int32_t speedV[LIGHTERS_AM];
int8_t angleSpeed[LIGHTERS_AM];
void lightersRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    randomSeed(millis());
    for (uint8_t i = 0U; i < LIGHTERS_AM; i++)
    {
      lightersPos[0U][i] = random(0, WIDTH * 10);
      lightersPos[1U][i] = random(0, HEIGHT * 10);
      lightersSpeed[0U][i] = random(-10, 10);
      lightersSpeed[1U][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0U, 255U), 255U, 255U);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20U) loopCounter = 0U;
  for (uint8_t i = 0U; i < modes[EFF_LIGHTERS].Scale; i++)
  {
    if (loopCounter == 0U)                                  // меняем скорость каждые 255 отрисовок
    {
      lightersSpeed[0U][i] += random(-3, 4);
      lightersSpeed[1U][i] += random(-3, 4);
      lightersSpeed[0U][i] = constrain(lightersSpeed[0U][i], -20, 20);
      lightersSpeed[1U][i] = constrain(lightersSpeed[1U][i], -20, 20);
    }

    lightersPos[0U][i] += lightersSpeed[0U][i];
    lightersPos[1U][i] += lightersSpeed[1U][i];

    if (lightersPos[0U][i] < 0) lightersPos[0U][i] = (WIDTH - 1) * 10;
    if (lightersPos[0U][i] >= (int32_t)(WIDTH * 10)) lightersPos[0U][i] = 0;

    if (lightersPos[1U][i] < 0)
    {
      lightersPos[1U][i] = 0;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
    }
    if (lightersPos[1U][i] >= (int32_t)(HEIGHT - 1) * 10)
    {
      lightersPos[1U][i] = (HEIGHT - 1U) * 10;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
    }
    drawPixelXY(lightersPos[0U][i] / 10, lightersPos[1U][i] / 10, lightersColor[i]);
  }
}

// ------------- светлячки со шлейфом -------------
int16_t coord[BALLS_AMOUNT][2U];
int8_t vector[BALLS_AMOUNT][2U];
CRGB ballColors[BALLS_AMOUNT];
void ballsRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;

    for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
    {
      int8_t sign;
      // забиваем случайными данными
      coord[j][0U] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0U] = random(4, 15) * sign;
      coord[j][1U] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1U] = random(4, 15) * sign;
      //ballColors[j] = CHSV(random(0, 9) * 28, 255U, 255U);
    }
  }

  if (!BALL_TRACK)                                          // режим без следов шариков
  {
    FastLED.clear();
  }
  else                                                      // режим со следами
  {
    fader(TRACK_STEP);
  }

  // движение шариков
  for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
  {
    // цвет зависит от масштаба
    ballColors[j] = CHSV((modes[EFF_LIGHTER_TRACES].Scale * (j + 1))%256U, 255U, 255U);
          
    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0U] > (int16_t)((WIDTH - 1) * 10))
    {
      coord[j][0U] = (WIDTH - 1) * 10;
      vector[j][0U] = -vector[j][0U];
    }
    if (coord[j][1U] > (int16_t)((HEIGHT - 1) * 10))
    {
      coord[j][1U] = (HEIGHT - 1) * 10;
      vector[j][1U] = -vector[j][1U];
    }
    leds[getPixelNumber(coord[j][0U] / 10, coord[j][1U] / 10)] =  ballColors[j];
  }
}

// ------------- пейнтбол -------------
void lightBallsRoutine(char *param)
{
  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw(beatsin8(3,64,100));
  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  // Use two out-of-sync sine waves
  uint16_t  i = beatsin16( 79, 0, 255); //91
  uint16_t  j = beatsin16( 67, 0, 255); //109
  uint16_t  k = beatsin16( 53, 0, 255); //73
  uint16_t  m = beatsin16( 97, 0, 255); //123

  // The color of each point shifts over time, each at a different speed.
  uint32_t ms = millis() / (modes[EFF_PAINTBALL].Scale/4 + 1);
  leds[getPixelNumber( highByte(i * paintWidth) + BORDERTHICKNESS, highByte(j * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 29, 200U, 255U);
  leds[getPixelNumber( highByte(j * paintWidth) + BORDERTHICKNESS, highByte(k * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 41, 200U, 255U);
  leds[getPixelNumber( highByte(k * paintWidth) + BORDERTHICKNESS, highByte(m * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 37, 200U, 255U);
  leds[getPixelNumber( highByte(m * paintWidth) + BORDERTHICKNESS, highByte(i * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 53, 200U, 255U);
}

// Trivial XY function for the SmartMatrix; use a different XY
// function for different matrix grids. See XYMatrix example for code.
uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;
  if (y & 0x01)
  {
    // Odd rows run backwards
    uint8_t reverseX = (WIDTH - 1) - x;
    i = (y * WIDTH) + reverseX;
  }
  else
  {
    // Even rows run forwards
    i = (y * WIDTH) + x;
  }
  return i%(WIDTH*HEIGHT+1);
}

// ------------- блуждающий кубик -------------
int16_t coordB[2U];
int8_t vectorB[2U];
CRGB ballColor;
int8_t ballSize;
void ballRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    //FastLED.clear();

    for (uint8_t i = 0U; i < 2U; i++)
    {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
      ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    }
  }

  ballSize = map(modes[EFF_CUBE].Scale, 0U, 255U, 2U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 2));
  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0)
    {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0U] > (int16_t)((WIDTH - ballSize) * 10))
  {
    coordB[0U] = (WIDTH - ballSize) * 10;
    vectorB[0U] = -vectorB[0U];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1U] > (int16_t)((HEIGHT - ballSize) * 10))
  {
    coordB[1U] = (HEIGHT - ballSize) * 10;
    vectorB[1U] = -vectorB[1U];
    if (RANDOM_COLOR)
    {
      ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    }
    //vectorB[1] += random(0, 6) - 3;
  }
  FastLED.clear();
  for (uint8_t i = 0U; i < ballSize; i++)
  {
    for (uint8_t j = 0U; j < ballSize; j++)
    {
      leds[getPixelNumber(coordB[0U] / 10 + i, coordB[1U] / 10 + j)] = ballColor;
    }
  }
}

// ------------- белый свет -------------
void whiteColorRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();

    for (uint16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(0U, 0U, 255U);
    }
  }
}

// ------------- белый свет (светится горизонтальная полоса по центру лампы; масштаб - высота центральной горизонтальной полосы; скорость - регулировка от холодного к тёплому; яркость - общая яркость) -------------
void whiteColorStripeRoutine(char *param)
{
    uint8_t centerY = max((uint8_t)round(HEIGHT / 2.0F) - 1, 0);
    uint8_t bottomOffset = (uint8_t)(!(HEIGHT & 1) && (HEIGHT > 1));                      // если высота матрицы чётная, линий с максимальной яркостью две, а линии с минимальной яркостью снизу будут смещены на один ряд
    for (int16_t y = centerY; y >= 0; y--)
    {
      CRGB color = CHSV(
        45U,                                                                              // определяем тон
        map(modes[EFF_WHITE_COLOR].Speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
        y == centerY                                                                                                    // определяем яркость
          ? BRIGHTNESS                                                                                                  // для центральной горизонтальной полосы (или двух) яркость всегда равна BRIGHTNESS
          : (modes[EFF_WHITE_COLOR].Scale / 100.0F) > ((centerY + 1.0F) - (y + 1.0F)) / (centerY + 1.0F) ? BRIGHTNESS : 0);  // для остальных горизонтальных полос яркость равна либо BRIGHTNESS, либо 0 в зависимости от масштаба //BRIGHTNESS/((centerY + 1.0F)-y)

      for (uint8_t x = 0U; x < WIDTH; x++)
      {
        drawPixelXY(x, y, color);                                                         // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
        drawPixelXY(x, max((uint8_t)(HEIGHT - 1U) - (y + 1U) + bottomOffset, 0U), color); // при нечётной - одна, но дважды
      }
    }
}

// ------------- мигающий цвет (не эффект! используется для отображения краткосрочного предупреждения; блокирующий код!) -------------
void showWarning(
  CRGB color,                                               /* цвет вспышки                                                 */
  uint32_t duration,                                        /* продолжительность отображения предупреждения (общее время)   */
  uint16_t blinkHalfPeriod)                                 /* продолжительность одной вспышки в миллисекундах (полупериод) */
{
  uint32_t blinkTimer = millis();
  enum BlinkState { OFF = 0, ON = 1 } blinkState = BlinkState::OFF;
  FastLED.setBrightness(WARNING_BRIGHTNESS);                // установка яркости для предупреждения
  FastLED.clear();
  delay(2);
  FastLED.show();

  for (uint16_t i = 0U; i < NUM_LEDS; i++)                  // установка цвета всех диодов в WARNING_COLOR
  {
    leds[i] = color;
  }

  uint32_t startTime = millis();
  while (millis() - startTime <= (duration + 5))            // блокировка дальнейшего выполнения циклом на время отображения предупреждения
  {
    if (millis() - blinkTimer >= blinkHalfPeriod)           // переключение вспышка/темнота
    {
      blinkTimer = millis();
      blinkState = (BlinkState)!blinkState;
      FastLED.setBrightness(blinkState == BlinkState::OFF ? 0 : WARNING_BRIGHTNESS);
      delay(1);
      FastLED.show();
    }
    delay(50);
  }

  FastLED.clear();
  FastLED.setBrightness(ONflag ? modes[currentMode].Brightness : 0);  // установка яркости, которая была выставлена до вызова предупреждения
  delay(1);
  FastLED.show();
  loadingFlag = true;                                       // принудительное отображение текущего эффекта (того, что был активен перед предупреждением)
}

//---------------------------------------------------------------------------
//-- 3D Noise эффектцы --------------
void madnessNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    scale = modes[EFF_MADNESS].Scale;
    speed = modes[EFF_MADNESS].Speed;
  }
  fillnoise8();
  for (uint8_t i = 0; i < WIDTH; i++)
  {
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      CRGB thisColor = CHSV(noise[j][i], 255, noise[i][j]);
      drawPixelXY(i, j, thisColor);                         //leds[getPixelNumber(i, j)] = CHSV(noise[j][i], 255, noise[i][j]);
    }
  }
  ihue += 1;
}

void rainbowNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = RainbowColors_p;
    scale = modes[EFF_RAINBOW].Scale;
    speed = modes[EFF_RAINBOW].Speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}

void rainbowStripeNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = RainbowStripeColors_p;
    scale = modes[EFF_RAINBOW_STRIPE].Scale;
    speed = modes[EFF_RAINBOW_STRIPE].Speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}

void zebraNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    // 'black out' all 16 palette entries...
    fill_solid(currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    scale = modes[EFF_ZEBRA].Scale;
    speed = modes[EFF_ZEBRA].Speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}

void forestNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = ForestColors_p;
    scale = modes[EFF_FOREST].Scale;
    speed = modes[EFF_FOREST].Speed;
    colorLoop = 0;
  }
  fillNoiseLED();
}

void oceanNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = OceanColors_p;
    scale = modes[EFF_OCEAN].Scale;
    speed = modes[EFF_OCEAN].Speed;
    colorLoop = 0;
  }

  fillNoiseLED();
}

void plasmaNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = PartyColors_p;
    scale = modes[EFF_PLASMA].Scale;
    speed = modes[EFF_PLASMA].Speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}

void cloudsNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = CloudColors_p;
    scale = modes[EFF_CLOUDS].Scale;
    speed = modes[EFF_CLOUDS].Speed;
    colorLoop = 0;
  }
  fillNoiseLED();
}

void lavaNoiseRoutine(char *param)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    currentPalette = LavaColors_p;
    scale = modes[EFF_LAVA].Scale;
    speed = modes[EFF_LAVA].Speed;
    colorLoop = 0;
  }
  fillNoiseLED();
}

// ************* СЛУЖЕБНЫЕ *************
void fillNoiseLED()
{
  uint8_t dataSmoothing = 0;
  if (speed < 50)
  {
    dataSmoothing = 200 - (speed * 4);
  }
  for (uint8_t i = 0; i < MAX_DIMENSION; i++)
  {
    int32_t ioffset = scale * i;
    for (uint8_t j = 0; j < MAX_DIMENSION; j++)
    {
      int32_t joffset = scale * j;

      uint8_t data = inoise8(x + ioffset, y + joffset, z);

      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if (dataSmoothing)
      {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }

      noise[i][j] = data;
    }
  }
  z += speed;

  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

  for (uint8_t i = 0; i < WIDTH; i++)
  {
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];
      // if this palette is a 'loop', add a slowly-changing base value
      if ( colorLoop)
      {
        index += ihue;
      }
      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if ( bri > 127 )
      {
        bri = 255;
      }
      else
      {
        bri = dim8_raw( bri * 2);
      }
      CRGB color = ColorFromPalette( currentPalette, index, bri);      
      drawPixelXY(i, j, color);                             //leds[getPixelNumber(i, j)] = color;
    }
  }
  ihue += 1;
}

void fillnoise8()
{
  for (uint8_t i = 0; i < MAX_DIMENSION; i++)
  {
    int32_t ioffset = scale * i;
    for (uint8_t j = 0; j < MAX_DIMENSION; j++)
    {
      int32_t joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset, y + joffset, z);
    }
  }
  z += speed;
}
