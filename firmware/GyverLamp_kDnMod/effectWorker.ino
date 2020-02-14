#ifdef NEWYEAR_MESSAGE
void NewYearMessagePrint()
{
      if(tmStringMoveTimeout.isReadyManual()){
        if(!isStrPrepated){
          time_t currentLocalTime = localTimeZone.toLocal(timeClient.getEpochTime());
          time_t nyLocalTime = localTimeZone.toLocal(NEWYEAR_UNIXDATETIME);
          calc = nyLocalTime - currentLocalTime; // unix_diff_time
          if(calc<0) {
            sprintf_P(strMessage, msg2, year(currentLocalTime));
          } else if(calc<300){
            sprintf_P(strMessage, msg1, (int)calc, "секунд");
          } else if(calc/60<60){
            sprintf_P(strMessage, msg1, (int)(calc/60), "минут");
          } else if(calc/(60*60)<60){
            sprintf_P(strMessage, msg1, (int)(calc/(60*60)), "часов");
          } else {
            byte calcN=(int)(calc/(60*60*24))%10; // остаток от деления на 10
            String str;
            if(calcN>=2 && calcN<=4)
              str = "дня";
            else if(calc!=11 && calcN==1)
              str = "день";
            else
              str = "дней";
            sprintf_P(strMessage, msg1, (int)(calc/(60*60*24)), str.c_str());
          }
          isStrPrepated = true;
          #ifdef GENERAL_DEBUG
          LOG.printf_P(PSTR("Prepared message: %s\n"), strMessage);
          #endif
        }
        
        if(tmStringMoveStep.isReadyManual()){
          if(!fillStringManual(strMessage, CRGB::White, false)) // смещаем
            tmStringMoveStep.reset();
          else {
            tmStringMoveTimeout.reset();
            isStrPrepated = false;
          }
        } else {
          fillStringManual(strMessage, CRGB::White, true); // выводим на том же месте
        }
      }
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
