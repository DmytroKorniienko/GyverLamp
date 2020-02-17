#pragma once

struct AlarmType
{
  bool State = false;
  uint16_t Time = 0U;
};

struct ModeType
{
  uint8_t Brightness = 50U;
  uint8_t Speed = 30U;
  uint8_t Scale = 40U;
};

typedef void (*SendCurrentDelegate)(char *outputBuffer);
typedef void (*ShowWarningDelegate)(CRGB color, uint32_t duration, uint16_t blinkHalfPeriod);
extern void showWarning(CRGB color,uint32_t duration,uint16_t blinkHalfPeriod);

// --------------------------------------

enum EFF_ENUM {                               // список и номера эффектов ниже в списке согласованы с android приложением! (не актуально, если приложение вычитывает список эффектов по EFF_ENUM)
EFF_SPARKLES = (0U),                          // Конфетти
EFF_FIRE,                                     // Огонь
//EFF_WHITTE_FIRE,                              // Белый огонь
EFF_WATERFALL,                                // Водопад
EFF_RAINBOW_VER,                              // Радуга вертикальная
EFF_RAINBOW_HOR,                              // Радуга горизонтальная
EFF_RAINBOW_DIAG,                             // Радуга диагональная
EFF_COLORS,                                   // Смена цвета
EFF_MADNESS,                                  // Безумие 3D
EFF_CLOUDS,                                   // Облака 3D
EFF_LAVA,                                     // Лава 3D
EFF_PLASMA,                                   // Плазма 3D
EFF_RAINBOW,                                  // Радуга 3D
EFF_RAINBOW_STRIPE,                           // Павлин 3D
EFF_ZEBRA,                                    // Зебра 3D
EFF_FOREST,                                   // Лес 3D
EFF_OCEAN,                                    // Океан 3D
EFF_COLOR,                                    // Цвет
EFF_SNOW,                                     // Снегопад
EFF_SNOWSTORM,                                // Метель
EFF_STARFALL,                                 // Звездопад
EFF_MATRIX,                                   // Матрица
EFF_LIGHTERS,                                 // Светлячки
EFF_LIGHTER_TRACES,                           // Светлячки со шлейфом
EFF_PAINTBALL,                                // Пейнтбол
EFF_CUBE,                                     // Блуждающий кубик
EFF_WHITE_COLOR                               // Белый свет (должен быть всегда последним для корректной работы рандомного демо-режима, т.к. он и все последующие будут исключены из обработки)
};

typedef struct _EFFECT {
  byte eff_nb;
  char *eff_name;
  byte min_brightness;
  byte max_brightness;
  byte min_speed;
  byte max_speed;
  byte min_scale;
  byte max_scale;  
  void (*func)(char*);
  char *param;
} EFFECT;

void sparklesRoutine(char*);
void fireRoutine(char*);
void rainbowVerticalRoutine(char*);
void rainbowHorizontalRoutine(char*);
void rainbowDiagonalRoutine(char*);
void colorsRoutine(char*);
void madnessNoiseRoutine(char*);
void cloudsNoiseRoutine(char*);
void lavaNoiseRoutine(char*);
void plasmaNoiseRoutine(char*);
void rainbowNoiseRoutine(char*);
void rainbowStripeNoiseRoutine(char*);
void zebraNoiseRoutine(char*);
void forestNoiseRoutine(char*);
void oceanNoiseRoutine(char*);
void colorRoutine(char*);
void snowRoutine(char*);
void snowStormRoutine(char*);
void starfallRoutine(char*);
void matrixRoutine(char*);
void lightersRoutine(char*);
void ballsRoutine(char*);
void lightBallsRoutine(char*);
void ballRoutine(char*);
void whiteColorStripeRoutine(char*);
void fire2012WithPalette(char*);

EFFECT EFFECTS_ARR[] = {
  {EFF_SPARKLES, "SPARKLES", 1, 255, 1, 255, 1, 255, sparklesRoutine, NULL},
  {EFF_FIRE, "FIRE", 1, 255, 1, 255, 1, 255, fireRoutine, "C"},
  //{EFF_WHITTE_FIRE, "WHITTE_FIRE", 1, 255, 1, 255, 1, 255, fireRoutine, "W"},
  {EFF_WATERFALL, "WATERFALL", 1, 255, 1, 255, 1, 255, fire2012WithPalette, NULL},
  {EFF_RAINBOW_VER, "RAINBOW_VER", 1, 255, 1, 255, 1, 255, rainbowVerticalRoutine, NULL},
  {EFF_RAINBOW_HOR, "RAINBOW_HOR", 1, 255, 1, 255, 1, 255, rainbowHorizontalRoutine, NULL},
  {EFF_RAINBOW_DIAG, "RAINBOW_DIAG", 1, 255, 1, 255, 1, 255, rainbowDiagonalRoutine, NULL},
  {EFF_COLORS, "COLORS", 1, 255, 1, 255, 1, 255, colorsRoutine, NULL},
  {EFF_MADNESS, "MADNESS", 1, 255, 1, 255, 1, 255, madnessNoiseRoutine, NULL},
  {EFF_CLOUDS, "CLOUDS", 1, 255, 1, 255, 1, 255, cloudsNoiseRoutine, NULL},
  {EFF_LAVA, "LAVA", 1, 255, 1, 255, 1, 255, lavaNoiseRoutine, NULL},
  {EFF_PLASMA, "PLASMA", 1, 255, 1, 255, 1, 255, plasmaNoiseRoutine, NULL},
  {EFF_RAINBOW, "RAINBOW", 1, 255, 1, 255, 1, 255, rainbowNoiseRoutine, NULL},
  {EFF_RAINBOW_STRIPE, "RAINBOW_STRIPE", 1, 255, 1, 255, 1, 255, rainbowStripeNoiseRoutine, NULL},
  {EFF_ZEBRA, "ZEBRA", 1, 255, 1, 255, 1, 255, zebraNoiseRoutine, NULL},
  {EFF_FOREST, "FOREST", 1, 255, 1, 255, 1, 255, forestNoiseRoutine, NULL},
  {EFF_OCEAN, "OCEAN", 1, 255, 1, 255, 1, 255, oceanNoiseRoutine, NULL},
  {EFF_COLOR, "COLOR", 1, 255, 1, 255, 1, 255, colorRoutine, NULL},
  {EFF_SNOW, "SNOW", 1, 255, 1, 255, 1, 255, snowRoutine, NULL},
  {EFF_SNOWSTORM, "SNOWSTORM", 1, 255, 1, 255, 1, 255, snowStormRoutine, NULL},
  {EFF_STARFALL, "STARFALL", 1, 255, 1, 255, 1, 255, starfallRoutine, NULL},
  {EFF_MATRIX, "MATRIX", 1, 255, 1, 255, 1, 255, matrixRoutine, NULL},
  {EFF_LIGHTERS, "LIGHTERS", 1, 255, 1, 255, 1, 255, lightersRoutine, NULL},
  {EFF_LIGHTER_TRACES, "LIGHTER_TRACES", 1, 255, 1, 255, 1, 255, ballsRoutine, NULL},
  {EFF_PAINTBALL, "PAINTBALL", 1, 255, 1, 255, 1, 255, lightBallsRoutine, NULL},
  {EFF_CUBE, "CUBE", 1, 255, 1, 255, 1, 255, ballRoutine, NULL},
  {EFF_WHITE_COLOR, "WHITE_COLOR", 1, 255, 1, 255, 1, 255, whiteColorStripeRoutine, NULL},
};

const int MODE_AMOUNT = sizeof(EFFECTS_ARR)/sizeof(EFFECT);     // количество режимов

typedef enum _LAMPMODE {
  MODE_NORMAL = 0,
  MODE_DEMO,
  MODE_WHITELAMP,
  MODE_ALARMCLOCK
} LAMPMODE;
