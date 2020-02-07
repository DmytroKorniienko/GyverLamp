//#define NEWYEAR_MESSAGE // вывод поздравления с новым годом и обратным отсчетом

static timerMinim tmTimeMoveStep(70); // скорость движения времени, в миллисекундах - меньше == быстрее
static timerMinim tmTimeCheckTimeout(10000); // период проверки времени (а дальше вывод в соответствии с расписанием), одного раза в десять секунд - достаточно, приоритет имеет вывод текста
                                             // для вывода текущего времени - стандартные настройки в константах PRINT_TIME


#ifdef NEWYEAR_MESSAGE
static timerMinim tmStringMoveStep(100); // скорость движения текста, в миллисекундах - меньше == быстрее
static timerMinim tmStringMoveTimeout(60000*5); // *N минут - периодичность вывода новогоднего отсчета/поздравления

char *msg1 = "До нового года осталось %d %s";
char *msg2 = "C новым 2020 годом!";
char message[256] = "До нового года осталось %d %s."; // буфер на 256 символов

void NewYearMessagePrint()
{
      #ifdef NEWYEAR_MESSAGE
      if(tmStringMoveTimeout.isReadyManual()){
        float calc;
        time_t currentLocalTime = localTimeZone.toLocal(timeClient.getEpochTime());
        time_t nyLocalTime = localTimeZone.toLocal(1577836800); // дата/время в UNIX формате, см. https://www.cy-pr.com/tools/time/
        calc = nyLocalTime - currentLocalTime; // unix_diff_time
        if(calc<0) {
          sprintf_P(message, msg2);
        } else if(calc<300){
          sprintf_P(message, msg1, (int)calc, "секунд");
        } else if(calc/60<60){
          sprintf_P(message, msg1, (int)calc/60, "минут");
        } else if(calc/60/60<60){
          sprintf_P(message, msg1, (int)calc/60/60, "часов");
        } else {
          sprintf_P(message, msg1, (int)calc/60/60/24, "дня");
        }
        
        if(tmStringMoveStep.isReadyManual()){
          if(!fillStringManual(message, CRGB::White, false)) // смещаем
            tmStringMoveStep.reset();
          else
            tmStringMoveTimeout.reset();
        } else {
          fillStringManual(message, CRGB::White, true);
        }
      }
      #endif
}
#endif

void onOffTimePrint()
{
    #ifdef NEWYEAR_MESSAGE
    if(tmTimeCheckTimeout.isReadyManual() && !tmStringMoveTimeout.isReadyManual()){ // в случае включенного новогоднего поздравления - не вклиниваемся в него пока выводится
    #else
    if(tmTimeCheckTimeout.isReadyManual()){
    #endif  
      if(tmTimeMoveStep.isReadyManual()){
        if(!printTime(thisTime, false, ONflag, true, false)) // проверка текущего времени и его вывод (если заказан и если текущее время соответстует заказанному расписанию вывода)
          tmTimeMoveStep.reset();
        else {
          tmTimeCheckTimeout.reset();
        }
      } else {
        printTime(thisTime, false, ONflag, true, true);
      }
      if(!ONflag)
        FastLED.show(); 
  }
}
