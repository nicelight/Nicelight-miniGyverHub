//setup()
void init_ntp() {

  NTP.begin(5);
//  NTP.begin();
  //  NTP.setGMT(5);
  //  NTP.setPeriod(600); // обновлять раз в 600 сек
}
//
////проверка NTP связи
//bool NTPerror() {
//  //проверим статус обновления ntp
//  byte ntpErr = NTP.status();
//  if (ntpErr) {
//    Serial.print("ntp err ");
//    Serial.println(ntpErr);
//  }
//  /* Код ошибок NTP
//    // 0 - всё ок
//    // 1 - не запущен UDP
//    // 2 - не подключен WiFi
//    // 3 - ошибка подключения к серверу
//    // 4 - ошибка отправки пакета
//    // 5 - таймаут ответа сервера
//    // 6 - получен некорректный ответ сервера
//  */
//  if (!NTP.synced()) Serial.println("NTP not sync");
//} //checkNTPstauts()
