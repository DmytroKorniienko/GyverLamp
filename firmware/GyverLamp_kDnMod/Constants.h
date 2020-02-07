#pragma once

#define ESP_USE_BUTTON                                      // если строка не закомментирована, должна быть подключена кнопка (иначе ESP может регистрировать "фантомные" нажатия и некорректно устанавливать яркость)
#define OTA                                                 // если строка не закомментирована, модуль будет ждать два последдовательных запроса пользователя на прошивку по воздуху (см. документацию в "шапке")
#define USE_NTP                                             // закомментировать или удалить эту строку, если нужно, чтобы устройство не лезло в интернет
#define USE_MQTT                                            // используется или нет mqtt клиент

// --- БИБЛИОТЕКИ ----------------------
#define FASTLED_INTERRUPT_RETRY_COUNT   (0)                 // default: 2; // Use this to determine how many times FastLED will attempt to re-transmit a frame if interrupted for too long by interrupts
//#define FASTLED_ALLOW_INTERRUPTS      (1)                 // default: 1; // Use this to force FastLED to allow interrupts in the clockless chipsets (or to force it to disallow), overriding the default on platforms that support this. Set the value to 1 to allow interrupts or 0 to disallow them.
#define FASTLED_ESP8266_RAW_PIN_ORDER                       // FASTLED_ESP8266_RAW_PIN_ORDER, FASTLED_ESP8266_D1_PIN_ORDER or FASTLED_ESP8266_NODEMCU_PIN_ORDER
//----------------------------------

#define ESP_MODE              (1U)                          // 0U - WiFi точка доступа, 1U - клиент WiFi (подключение к роутеру)
uint8_t espMode = ESP_MODE;                                 // ESP_MODE может быть сохранён в энергонезависимую память и изменён в процессе работы лампы без необходимости её перепрошивки
#define BRIGHTNESS            (255U)                        // стандартная маскимальная яркость (0-255)

// ============= НАСТРОЙКИ =============
// --- ESP -----------------------------
#define LED_PIN               (2U)                          // пин ленты                (D4)
#define BTN_PIN               (4U)                          // пин кнопки               (D2)
#define MOSFET_PIN            (5U)                          // пин MOSFET транзистора   (D1) - может быть использован для управления питанием матрицы/ленты
#define ALARM_PIN             (16U)                         // пин состояния будильника (D0) - может быть использован для управления каким-либо внешним устройством на время работы будильника
#define MOSFET_LEVEL          (HIGH)                        // логический уровень, в который будет установлен пин MOSFET_PIN, когда матрица включена - HIGH или LOW
#define ALARM_LEVEL           (HIGH)                        // логический уровень, в который будет установлен пин ALARM_PIN, когда "рассвет"/будильник включен

#define ESP_HTTP_PORT         (80U)                         // номер порта, который будет использоваться во время первой утановки имени WiFi сети (и пароля), к которой потом будет подключаться лампа в режиме WiFi клиента (лучше не менять)
#define ESP_UDP_PORT          (8888U)                       // номер порта, который будет "слушать" UDP сервер во время работы лампы как в режиме WiFi точки доступа, так и в режиме WiFi клиента (лучше не менять)

// --- МАТРИЦА -------------------------
#define MIN_WHITE_COLOR_BRGHT (1U)                          // минимальная яркость EFF_WHITE_COLOR режима, белая лампа (0-100) для исключения нерабочей зоны
#define CURRENT_LIMIT         (2500U)                       // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH                 (15U)                         // ширина матрицы
#define HEIGHT                (10U)                         // высота матрицы

#define COLOR_ORDER           (GRB)                         // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE           (0U)                          // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE      (0U)                          // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION       (1U)                          // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
                                                            // при неправильной настройке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
                                                            // шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/
#ifdef OTA
#define ESP_OTA_PORT          (8266U)                       // номер порта, который будет "прослушиваться" в ожидании команды прошивки по воздуху
#define CONFIRMATION_TIMEOUT  (30U)                         // время в сеундах, в течение которого нужно дважды подтвердить старт обновлениЯ по воздуху (иначе сброс в None)
#endif
//#define ESP_CONN_TIMEOUT      (7U)                          // время в секундах (ДОЛЖНО БЫТЬ МЕНЬШЕ 8, иначе сработает WDT), которое ESP будет пытаться подключиться к WiFi сети, после его истечения автоматически развернёт WiFi точку доступа
//#define ESP_CONF_TIMEOUT      (300U)                        // время в секундах, которое ESP будет ждать ввода SSID и пароля WiFi сети роутера в конфигурационном режиме, после его истечения ESP перезагружается
#define ESP_CONN_TIMEOUT      (15U)                         // новая реализация
#define ESP_CONF_TIMEOUT      (120U)                        // новая реализация
// --- AP (WiFi точка доступа) ---
#define AP_NAME               ("LedLamp")                   // имя WiFi точки доступа, используется как при запросе SSID и пароля WiFi сети роутера, так и при работе в режиме ESP_MODE = 0
#define AP_PASS               ("31415926")                  // пароль WiFi точки доступа
const uint8_t AP_STATIC_IP[] = {192, 168, 4, 1};            // статический IP точки доступа (лучше не менять)

//#define MAX_UDP_BUFFER_SIZE (UDP_TX_PACKET_MAX_SIZE + 1)
#define MAX_UDP_BUFFER_SIZE   (129U)                        // максимальный размер буффера UDP сервера

// --- FAVORITES ----------------------
#define DEFAULT_FAVORITES_INTERVAL           (300U)         // значение по умолчанию для интервала переключения избранных эффектов в секундах
#define DEFAULT_FAVORITES_DISPERSION         (0U)           // значение по умолчанию для разброса интервала переключения избранных эффектов в секундах
// --- ВНЕШНЕЕ УПРАВЛЕНИЕ --------------
#ifdef USE_MQTT
#define MQTT_RECONNECT_TIME   (10U)                         // время в секундах перед подключением к MQTT брокеру в случае потери подключения
static const char TopicBase[]          PROGMEM = "LedLamp";                     // базовая часть топиков
static const char TopicCmnd[]          PROGMEM = "cmnd";                        // часть командных топиков (входящие команды лампе)
static const char TopicState[]         PROGMEM = "state";                       // часть топиков состояния (ответ от лампы)

static const char MqttServer[]         PROGMEM = "192.168.0.100";               // строка с IP адресом MQTT брокера
static const uint16_t MqttPort                 = 1883U;                         // порт MQTT брокера
static const char MqttUser[]           PROGMEM = "";                            // пользователь MQTT брокера
static const char MqttPassword[]       PROGMEM = "";                            // пароль пользователя MQTT брокера
static const char MqttClientIdPrefix[] PROGMEM = "LedLamp_";                    // id клиента MQTT брокера (к нему будет добавлен ESP.getChipId)
#endif

//----------------------------------

#include <pgmspace.h>
#include <FastLED.h>
#include <WiFiManager.h>

//#include <ESP8266WebServer.h>
//#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
//#include <EEPROM.h>
#include <TimeLib.h>

#ifdef USE_NTP
#include <NTPClient.h>
#include <Timezone.h>
#endif
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
#include "Types.h"
#include "timerMinimManager.h"
#ifdef OTA
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include "OtaManager.h"
#endif
#ifdef USE_MQTT
#include <AsyncMqttClient.h>
#include "MqttManager.h"
#endif
#include "EepromManager.h"
#include "FavoritesManager.h"
#include "fonts.h"
#include "CaptivePortalManager.h"

// настройка кнопки
#ifdef ESP_USE_BUTTON
const byte PULL_MODE=HIGH_PULL;                             // подтяжка кнопки к питанию (для механических кнопок НО, на массу)
//const byte PULL_MODE=LOW_PULL;                            // подтяжка кнопки к нулю (для сенсорных кнопок на TP223)
GButton touch(BTN_PIN, PULL_MODE, NORM_OPEN);               

#define BUTTON_STEP_TIMEOUT   (100U)                        // каждые BUTTON_STEP_TIMEOUT мс будет генерироваться событие удержания кнопки (для регулировки яркости)
#define BUTTON_CLICK_TIMEOUT  (500U)                        // максимальное время между нажатиями кнопки в мс, до достижения которого считается серия последовательных нажатий
#define BUTTON_TIMEOUT        (700U)                        // с какого момента начинает считаться, что кнопка удерживается в мс
#define ESP_RESET_ON_START    (true)                        // true - если при старте нажата кнопка (или кнопки нет!), сохранённые настройки будут сброшены; false - не будут
#else
#define ESP_RESET_ON_START    (false)                       // true - если при старте нажата кнопка (или кнопки нет!), сохранённые настройки будут сброшены; false - не будут
#endif

// ============= ОСТАЛЬНЫЕ НАСТРОЙКИ =============
#define GENERAL_DEBUG                                       // если строка не закомментирована, будут выводиться отладочные сообщения
#define WIFIMAN_DEBUG         (true)                        // вывод отладочных сообщений при подключении к WiFi сети: true - выводятся, false - не выводятся; настройка не зависит от GENERAL_DEBUG
#define GENERAL_DEBUG_TELNET  (false)                       // true - отладочные сообщения будут выводиться в telnet вместо Serial порта (для удалённой отладки без подключения usb кабелем)
#define TELNET_PORT           (23U)                         // номер telnet порта

// --- ESP (WiFi клиент) ---------------
const uint8_t STA_STATIC_IP[] = {};                         // статический IP адрес: {} - IP адрес определяется роутером; {192, 168, 1, 66} - IP адрес задан явно (если DHCP на роутере не решит иначе); должен быть из того же диапазона адресов, что разадёт роутер
                                                            // SSID WiFi сети и пароль будут запрошены WiFi Manager'ом в режиме WiFi точки доступа, нет способа захардкодить их в прошивке

// --- ВРЕМЯ ---------------------------
#ifdef USE_NTP

#define RESOLVE_INTERVAL      (5UL * 60UL * 1000UL)                       // интервал проверки подключения к интеренету в миллисекундах (5 минут)
                                                                          // при старте ESP пытается получить точное время от сервера времени в интрнете
                                                                          // эта попытка длится RESOLVE_TIMEOUT
                                                                          // если при этом отсутствует подключение к интернету (но есть WiFi подключение),
                                                                          // модуль будет подвисать на RESOLVE_TIMEOUT каждое срабатывание таймера, т.е., 3 секунды
                                                                          // чтобы избежать этого, будем пытаться узнать состояние подключения 1 раз в RESOLVE_INTERVAL (5 минут)
                                                                          // попытки будут продолжаться до первой успешной синхронизации времени
                                                                          // до этого момента функции будильника работать не будут
                                                                          // интервал последующих синхронизаций времени определяён в NTP_INTERVAL (30 минут)
                                                                          // при ошибках повторной синхронизации времени функции будильника отключаться не будут
#define RESOLVE_TIMEOUT       (1500UL)                                    // таймаут ожидания подключения к интернету в миллисекундах (1,5 секунды)

#define NTP_ADDRESS           ("ntp2.colocall.net")         // сервер времени
#define NTP_INTERVAL          (30UL * 60UL * 1000UL)        // интервал синхронизации времени (30 минут)
#endif
//#define SUMMER_WINTER_TIME                                  // для тех, кому нужен переход на зимнее/летнее время - оставить строку, остальным - закомментировать или удалить
#ifndef SUMMER_WINTER_TIME
#define LOCAL_WEEK_NUM        (week_t::Last)                // для стран, где нет перехода на зимнее/летнее время это технический параметр, не нужно его изменять
#define LOCAL_WEEKDAY         (dow_t::Sun)                  // для стран, где нет перехода на зимнее/летнее время это технический параметр, не нужно его изменять
#define LOCAL_MONTH           (month_t::Mar)                // для стран, где нет перехода на зимнее/летнее время это технический параметр, не нужно его изменять
#define LOCAL_HOUR            (1U)                          // для стран, где нет перехода на зимнее/летнее время это технический параметр, не нужно его изменять
#define LOCAL_OFFSET          (2 * 60)                      // смещение локального времени относительно универсального координированного времени UTC в минутах
#define LOCAL_TIMEZONE_NAME   ("KYIV")                       // обозначение локального часового пояса; до 5 символов; может быть использовано, если понадобится его вывести после вывода времени
#else
#define SUMMER_WEEK_NUM       (week_t::Last)                // номер недели в месяце, когда происходит переход на летнее время (возможные варианты: First - первая, Second - вторая, Third - третья, Fourth - четвёртая, Last - последняя)
#define SUMMER_WEEKDAY        (dow_t::Sun)                  // день недели, когда происходит переход на летнее время (возможные варианты: Mon - пн, Tue - вт, Wed - ср, Thu - чт, Sat - сб, Sun - вс)
#define SUMMER_MONTH          (month_t::Mar)                // месяц, в котором происходит переход на летнее время (возможные варианты: Jan - январь, Feb - февраль, Mar - март, Apr - апрель, May - май, Jun - июнь, Jul - июль, Aug - август, Sep - сентябрь, Oct - октябрь, Nov - ноябрь, Dec - декабрь)
#define SUMMER_HOUR           (3U)                          // час (по зимнему времени!), когда заканчивается зимнее время и начинается летнее; [0..23]
#define SUMMER_OFFSET         (3 * 60)                      // смещение летнего времени относительно универсального координированного времени UTC в минутах
#define SUMMER_TIMEZONE_NAME  ("EEST")                      // обозначение летнего времени; до 5 символов; может быть использовано, если понадобится его вывести после вывода времени; может быть "ЛЕТ"
#define WINTER_WEEK_NUM       (week_t::Last)                // номер недели в месяце, когда происходит переход на зимнее время (возможные варианты: First - первая, Second - вторая, Third - третья, Fourth - четвёртая, Last - последняя)
#define WINTER_WEEKDAY        (dow_t::Sun)                  // день недели, когда происходит переход на зимнее время (возможные варианты: Mon - пн, Tue - вт, Wed - ср, Thu - чт, Sat - сб, Sun - вс)
#define WINTER_MONTH          (month_t::Oct)                // месяц, в котором происходит переход на зимнее время (возможные варианты: Jan - январь, Feb - февраль, Mar - март, Apr - апрель, May - май, Jun - июнь, Jul - июль, Aug - август, Sep - сентябрь, Oct - октябрь, Nov - ноябрь, Dec - декабрь)
#define WINTER_HOUR           (3U)                          // час (по летнему времени!), когда заканчивается летнее время и начинается зимнее; [0..23]
#define WINTER_OFFSET         (2 * 60)                      // смещение зимнего времени относительно универсального координированного времени UTC в минутах
#define WINTER_TIMEZONE_NAME  ("EET")                       // обозначение зимнего времени; до 5 символов; может быть использовано, если понадобится его вывести после вывода времени; может быть "ЗИМ"
#endif
#define NIGHT_HOURS_START     (1380U)                       // начало действия "ночного времени" (в минутах от начала суток, 23:00), текущее время бегущей строкой будет выводиться с яркостью NIGHT_HOURS_BRIGHTNESS
#define NIGHT_HOURS_STOP      (479U)                        // конец действия "ночного времени" (в минутах от начала суток, 7:59)
#define DAY_HOURS_BRIGHTNESS  (255)                         // яркость для вывода текущего времени бегущей строкой днём; если -1, будет использована яркость текущего эффекта (она известна, даже когда матрица выключена)
#define NIGHT_HOURS_BRIGHTNESS (5)                          // яркость для вывода текущего времени бегущей строкой ночью; если -1, будет использована яркость текущего эффекта (она известна, даже когда матрица выключена)
                                                            // константы DAY_HOURS_BRIGHTNESS и NIGHT_HOURS_BRIGHTNESS используются только, когда матрица выключена, иначе будет использована яркость текущего эффекта
#define PRINT_TIME            (6U)                          // 0U - не выводить время бегущей строкой; 1U - вывод времени каждый час; 2U - каждый час + каждые 30 минут; 3U - каждый час + каждые 15 минут
                                                            // 4U - каждый час + каждые 10 минут; 5U - каждый час + каждые 5 минут; 6U - каждый час + каждую минуту
// --- РАССВЕТ -------------------------
#define DAWN_BRIGHT           (200U)                        // максимальная яркость рассвета (0-255)
#define DAWN_TIMEOUT          (1U)                          // сколько рассвет светит после времени будильника, минут

#define NUMHOLD_TIME          (1500U)                       // время отображения индикатора уровня яркости/скорости/масштаба в мс
#define VERTGAUGE             (1U)                          // вертикальный/горизонтальный/отключен (1/2/0) индикатор
#define DEMOTIME              (30U)                         // в секундах
#define RANDOM_DEMO           (1)                           // 0,1 - рандомный выбор режима демо
// ============= ДЛЯ РАЗРАБОТЧИКОВ =====

#if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
WiFiServer telnetServer(TELNET_PORT);                       // telnet сервер
WiFiClient telnet;                                          // обработчик событий telnet клиента
bool telnetGreetingShown = false;                           // признак "показано приветствие в telnet"
#define LOG                   telnet
#else
#define LOG                   Serial
#endif

#define NUM_LEDS              (uint16_t)(WIDTH * HEIGHT)
#define SEGMENTS              (1U)                          // диодов в одном "пикселе" (для создания матрицы из кусков ленты)

// работа с бегущим текстом
// --- НАСТРОЙКИ ТЕКСТА ----------------
#define TEXT_DIRECTION        (1U)                          // 1 - по горизонтали, 0 - по вертикали
#define MIRR_V                (1U)                          // отразить текст по вертикали (0 / 1)
#define MIRR_H                (0U)                          // отразить текст по горизонтали (0 / 1)
//#define ROTATE                (1U)                          // повернуть символ на 90 градусов (0 / 1)

#define TEXT_HEIGHT           (1U)                          // высота, на которой бежит текст (от низа матрицы)
#define LET_WIDTH             (5U)                          // ширина буквы шрифта
#define LET_HEIGHT            (8U)                          // высота буквы шрифта
#define SPACE                 (1U)                          // пробел
#define LETTER_COLOR          (CRGB::White)                 // цвет букв по умолчанию


// --- НАСТРОЙКИ ЭФФЕКТОВ --------------
// ------------- конфетти --------------
#define FADE_OUT_SPEED        (70U)                         // скорость затухания
// ------------- огонь -----------------
#define SPARKLES              (1U)                          // вылетающие угольки вкл выкл
// ------------- метель -------------
#define SNOW_DENSE            (60U)                         // плотность снега
#define SNOW_TAIL_STEP        (100U)                        // длина хвоста
#define SNOW_SATURATION       (0U)                          // насыщенность (от 0 до 255)
// ------------- звездопад -------------
#define STAR_DENSE            (60U)                         // плотность комет
#define STAR_TAIL_STEP        (100U)                        // длина хвоста кометы
#define STAR_SATURATION       (150U)                        // насыщенность кометы (от 0 до 255)
// ------------- светлячки --------------
#define LIGHTERS_AM           (100U)
// ------------- светлячки со шлейфом -------------
#define BALLS_AMOUNT          (3U)                          // количество "шариков"
#define CLEAR_PATH            (1U)                          // очищать путь
#define BALL_TRACK            (1U)                          // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP            (70U)                         // длина хвоста шарика (чем больше цифра, тем хвост короче)
// ------------- пейнтбол -------------
#define BORDERTHICKNESS       (1U)                          // глубина бордюра для размытия яркой частицы: 0U - без границы (резкие края); 1U - 1 пиксель (среднее размытие) ; 2U - 2 пикселя (глубокое размытие)
const uint8_t paintWidth = WIDTH - BORDERTHICKNESS * 2;
const uint8_t paintHeight = HEIGHT - BORDERTHICKNESS * 2;
// ------------- блуждающий кубик -------------
#define RANDOM_COLOR          (1U)                          // случайный цвет при отскоке
// ------------- мигающий цвет (не эффект! используется для отображения краткосрочного предупреждения; блокирующий код!) -------------
#define WARNING_BRIGHTNESS    (10U)                         // яркость вспышки

//-----------------------------------------------------------------
// --- ИНИЦИАЛИЗАЦИЯ ОБЪЕКТОВ ----------
CRGB leds[NUM_LEDS];
WiFiManager wifiManager;
WiFiServer wifiServer(ESP_HTTP_PORT);
WiFiUDP Udp;

#ifdef USE_NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL); // объект, запрашивающий время с ntp сервера; в нём смещение часового пояса не используется (перенесено в объект localTimeZone); здесь всегда должно быть время UTC
  #ifdef SUMMER_WINTER_TIME
  TimeChangeRule summerTime = { SUMMER_TIMEZONE_NAME, SUMMER_WEEK_NUM, SUMMER_WEEKDAY, SUMMER_MONTH, SUMMER_HOUR, SUMMER_OFFSET };
  TimeChangeRule winterTime = { WINTER_TIMEZONE_NAME, WINTER_WEEK_NUM, WINTER_WEEKDAY, WINTER_MONTH, WINTER_HOUR, WINTER_OFFSET };
  Timezone localTimeZone(summerTime, winterTime);
  #else
  TimeChangeRule localTime = { LOCAL_TIMEZONE_NAME, LOCAL_WEEK_NUM, LOCAL_WEEKDAY, LOCAL_MONTH, LOCAL_HOUR, LOCAL_OFFSET };
  Timezone localTimeZone(localTime);
  #endif
#endif

#define PRINT_ALARM_TIME
#define TEXT_SCROLL_SPEED 100           // скорость прокрутки в мс, чем меньше - тем быстрее прокрутка
timerMinim timeTimer(20*1000);          // раз в N секунд будет проверяться время в будильнике для смены яркости и выводиться время, если не закомментировано PRINT_ALARM_TIME 
bool ntpServerAddressResolved = false;
bool timeSynched = false;
uint32_t lastTimePrinted = 0U;

#ifdef OTA
OtaManager otaManager(&showWarning);
OtaPhase OtaManager::OtaFlag = OtaPhase::None;
#endif

#ifdef USE_MQTT
AsyncMqttClient* mqttClient = NULL;
AsyncMqttClient* MqttManager::mqttClient = NULL;
char* MqttManager::mqttServer = NULL;
char* MqttManager::mqttUser = NULL;
char* MqttManager::mqttPassword = NULL;
char* MqttManager::clientId = NULL;
char* MqttManager::lampInputBuffer = NULL;
char* MqttManager::topicInput = NULL;
char* MqttManager::topicOutput = NULL;
bool MqttManager::needToPublish = false;
char MqttManager::mqttBuffer[] = {};
uint32_t MqttManager::mqttLastConnectingAttempt = 0;
SendCurrentDelegate MqttManager::sendCurrentDelegate = NULL;
#endif

// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_UDP_BUFFER_SIZE];                     // buffer to hold incoming packet
char inputBuffer[MAX_UDP_BUFFER_SIZE];
static const uint8_t maxDim = max(WIDTH, HEIGHT);
String host_name = String(AP_NAME) + "_" + String(ESP.getChipId(),HEX);             // установка универсального имени хоста/сети

ModeType modes[MODE_AMOUNT];
AlarmType alarms[7];

static const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60};   // опции для выпадающего списка параметра "время перед 'рассветом'" (будильник); синхронизировано с android приложением
uint8_t dawnMode;
bool dawnFlag = false;
uint32_t thisTime;
bool manualOff = false;

int8_t currentMode = 0;
bool loadingFlag = true;
bool ONflag = false;
uint32_t eepromTimeout;
bool settChanged = false;
bool buttonEnabled = true;
bool needconfigure = false;
bool isWifiOffMode = false;
#ifdef GENERAL_DEBUG
static const char * enumConnectionStatus[] = { "WL_IDLE_STATUS", "WL_NO_SSID_AVAIL", "WL_SCAN_COMPLETED", "WL_CONNECTED", "WL_CONNECT_FAILED", "WL_CONNECTION_LOST", "WL_DISCONNECTED" }; //перечисление статусов сети
#endif
unsigned char matrixValue[8][16];

bool TimerManager::TimerRunning = false;
bool TimerManager::TimerHasFired = false;
uint8_t TimerManager::TimerOption = 1U;
uint64_t TimerManager::TimeToFire = 0ULL;

uint8_t FavoritesManager::FavoritesRunning = 0;
uint16_t FavoritesManager::Interval = DEFAULT_FAVORITES_INTERVAL;
uint16_t FavoritesManager::Dispersion = DEFAULT_FAVORITES_DISPERSION;
uint8_t FavoritesManager::UseSavedFavoritesRunning = 0;
uint8_t FavoritesManager::FavoriteModes[MODE_AMOUNT]; // static = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint32_t FavoritesManager::nextModeAt = 0UL;

bool CaptivePortalManager::captivePortalCalled = false;

byte GlobalBrightness = BRIGHTNESS; // глобальная яркость, пока что будет использоваться для демо-режимов
byte numHold = 0;
byte lampMode = MODE_NORMAL;
byte storeEffBrightness = 127;
bool brightDirection, speedDirection, scaleDirection;
static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой
static bool setDirectionTimeout = false;                    // флаг: начало отсчета таймаута на смену направления регулировки
static bool isFirstHoldingPress = false;                    // флаг: только начали удерживать?
static timerMinim tmNumHoldTimer(NUMHOLD_TIME);             // таймаут удержания кнопки в мс
static timerMinim tmUserTimer(DEMOTIME*1000);               // смена эффекта в демо режиме по дабл-клику из выключенного состояния, таймаут DEMOTIME секунд
static timerMinim tmChangeDirectionTimer(NUMHOLD_TIME);      // таймаут смены направления увеличение-уменьшение при удержании кнопки
byte storeEffect = 1;
byte storeMode = MODE_NORMAL;
