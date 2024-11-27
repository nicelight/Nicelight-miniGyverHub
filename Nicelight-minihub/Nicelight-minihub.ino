
//D:\nicelight_HDD_2021\projects\Hydroponic\Hydro-Settings-goroshek\Nicelight-miniGyverHub\Nicelight-minihub

// имя пароль вашей домашней сети
// можно ввести, подключившись к ESP AP c паролем 1234567890
#define WIFI ""
#define WIFIPASS ""
#define INDIKATOR 2         // на каком пине индикаторный светодиод




#include <Arduino.h>
#include "timer.h"
#include "led.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <GyverDBFile.h>
#include <LittleFS.h>
GyverDBFile db(&LittleFS, "/nicelight.db"); // база данных для хранения настроек будет автоматически записываться в файл при изменениях

#include <SettingsGyver.h>
#include <WiFiConnector.h>
SettingsGyver sett("Горошек для любимой", &db);// указывается заголовок меню, подключается база данных


#include <GyverNTP.h>

//обявление фкнций для их видимости из вкладок.
void build(sets::Builder& b);
void update(sets::Updater& upd);

//GPdate nowDate;
/* как пользоваться:
   Установка и считывание даты:
  nowDate.year = 2024;
  nowDate.month = 1;
  nowDate.day = 20;
  nowDate.set(2024, 1, 20);
*/

//GPtime nowTime;
//GPtime startTime;
//GPtime stopTime;
/* как пользоваться:
   Установка и считывание времени:
  nowTime.hour = 10;
  nowTime.minute = 25;
  nowTime.second = 58;
  nowTime.set(12, 30, 0);
*/

int valNum;

uint32_t startSeconds = 0;
uint32_t stopSeconds = 0;

bool notice_f; // флаг на отправку уведомления о подключении к wifi


// ключи для хранения в базе данных они же идентификаторы полей для веб морды
enum kk : size_t {
  txt,
  pass,
  uintw,
  intw,
  int64w,

  color,
  toggle,
  slider,
  selectw,

  lbl1,
  lbl2,

  date,
  t1f1_startTime,
  datime,
  secondsNow,

  btn1,
  btn2,
  wifi_ssid,
  wifi_pass,
  apply
};

//данные для работы бизнес логики
struct Data {
  uint32_t secondsNow = 12 * 3600 + 30 * 60;
  // String label = "label";
  // bool led = 1;
  //    String paragr = "paragraph";
  //    String input = "input";
  //    char inputc[20] = "input";
  //    int number = 123456;
  //    String pass = "pass";
  //    uint32_t color = 0xff0000;
  //    bool sw = true;
  //    uint32_t datew = 1728123055;
  //    uint32_t datetime = 1728123055;
  //    float slider = 33;
  //    uint8_t sel = 1;
};
Data data;

Timer each60Sec(60000); // таймер раз в минуту

LED indikator(INDIKATOR, 300, 3, 50, 20); //каждые 1000 милисек мигаем 3 раза каждых 50 мс, время горения 20 мсек



// билдер! Тут строится наше окно настроек



void setup() {
  Serial.begin(115200);
  Serial.println();

  // ======== SETTINGS ========
  WiFi.mode(WIFI_AP_STA);   // режим AP_STA. Вызываем перед sett.begin(), чтобы settings знал о текущем режиме wifi
  sett.begin();
  sett.onBuild(build);
  sett.onUpdate(update);

  // ======== DATABASE ========
#ifdef ESP32
  LittleFS.begin(true);
#else
  LittleFS.begin();
#endif
  db.begin();
  //  db.init(kk::txt, "text");   // инициализация базы данных начальными значениями
  //  db.init(kk::pass, "some pass");
  //  db.init(kk::uintw, 64u);
  //  db.init(kk::intw, -10);
  //  db.init(kk::int64w, 1234567ll);
  //  db.init(kk::color, 0xff0000);
  //  db.init(kk::toggle, (bool)1);
  //  db.init(kk::selectw, (uint8_t)1);
  //  db.init(kk::date, 1719941932);
  //  db.init(kk::datime, 1719941932);
  db.init(kk::t1f1_startTime, 60);
  db.init(kk::slider, -3.5);

  db.init(kk::wifi_ssid, WIFI);
  db.init(kk::wifi_pass, WIFIPASS);
  db.dump(Serial);

  // ======== WIFI ========
  // подключение и реакция на подключение или ошибку
  WiFiConnector.setPass("1234567890"); // пароль точки доступа
  WiFiConnector.setTimeout(10); // сколько секунд пытаться приконнектиттся
  WiFiConnector.onConnect([]() {
    Serial.print("Connected! ");
    Serial.println(WiFi.localIP());
    indikator.setPeriod(3000, 1, 200, 150); //раз в 000 сек, 0 раз взмигнем - по 00 милисек периоды, гореть будем 0 милисек
  });
  WiFiConnector.onError([]() {
    Serial.print("Error! start AP ");
    Serial.println(WiFi.softAPIP());
    indikator.setPeriod(600, 2, 100, 50); //раз в  секунду два раза взмигнем - по 200 милисек, гореть будем 50 милисек

  });

  WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);

  NTP.begin(5); // часовой пояс. Для Москвы: 3. Худжанд, Нск 5. Обновляться раз в 600 сек
  NTP.setPeriod(32000); // обновлять раз в 600 сек
}//setup


void loop() {
  WiFiConnector.tick(); // поддержка wifi связи
  sett.tick();  // поддержка веб интерфейса
  NTP.tick(); // поддержка NTP
  indikator.tick(); // in loop

  if (each60Sec.ready()) { // раз в минуту
    if (!NTP.status() && NTP.synced()) {
      // берем текущую дату и время
      //db[kk::t1f1_startTime] =  //тут хранится время
      //    nowTime.set(ntp.hour(), ntp.minute(), ntp.second());
      //    nowDate.set(ntp.year(), ntp.month(), ntp.day());
      data.secondsNow = NTP.daySeconds();
    } else Serial.print("NTP discon");

  }//each60Sec

  if (NTP.newSecond()) {
    data.secondsNow++;
  }
}//loop
