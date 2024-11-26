#include <Arduino.h>

#define WIFI_SSID ""
#define WIFI_PASS ""

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <GyverDBFile.h>
#include <LittleFS.h>

// база данных для хранения настроек
// будет автоматически записываться в файл при изменениях
GyverDBFile db(&LittleFS, "/data.db");

#include <SettingsGyver.h>
#include <WiFiConnector.h>

// указывается заголовок меню, подключается база данных
SettingsGyver sett("Горошек для любимой", &db);

bool notice_f; // флаг на отправку уведомления о подключении к wifi


// ключи для хранения в базе данных
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
  timew,
  datime,

  btn1,
  btn2,
  wifi_ssid,
  wifi_pass,
  apply
};

// билдер! Тут строится наше окно настроек
void build(sets::Builder& b) {
  // можно узнать, было ли действие по виджету
  if (b.build.isAction()) {
    Serial.print("Set: 0x");
    Serial.print(b.build.id, HEX);
    Serial.print(" = ");
    Serial.println(b.build.value);
  }

  // группа. beginGroup всегда вернёт true для удобства организации кода
  if (b.beginGroup("Group 1")) {
    b.Input(kk::txt, "Text");
    b.Pass(kk::pass, "Password");
    b.Input(kk::uintw, "uint");
    b.Input(kk::intw, "int");
    b.Input(kk::int64w, "int 64");

    b.endGroup();  // НЕ ЗАБЫВАЕМ ЗАВЕРШИТЬ ГРУППУ
  }

  // пара лейблов вне группы. Так тоже можно
  b.Label(kk::lbl1, "Random");
  b.Label(kk::lbl2, "millis()", "", sets::Colors::Red);

  // ещё группа
  // можно использовать такой синтаксис: sets::Group(Builder&, title) заключается в блок кода в самом начале,
  // вызывать endGroup в этом случае не нужно
  // так же работают остальные контейнеры (Menu, Buttons)
  {
    sets::Group g(b, "Group 2");
    b.Color(kk::color, "Color");
    b.Switch(kk::toggle, "Switch");
    b.Select(kk::selectw, "Select", "var1;var2;hello");
    b.Slider(kk::slider, "Slider", -10, 10, 0.5, "deg");
  }

  // и ещё
  if (b.beginGroup("Group3")) {
    b.Date(kk::date, "Date");
    b.Time(kk::timew, "Time");
    b.DateTime(kk::datime, "Datime");

    // а это кнопка на вложенное меню. Далее нужно описать его содержимое
    if (b.beginMenu("Подменю")) {
      // тут тоже могут быть группы
      if (b.beginGroup("Group 3")) {
        b.Switch("sw1"_h, "switch 1");
        b.Switch("sw2"_h, "switch 2");
        b.Switch("sw3"_h, "switch 3");
        b.endGroup();
      }

      {
        sets::Group g(b, "настройки WiFi");
        b.Input(kk::wifi_ssid, "SSID");
        b.Pass(kk::wifi_pass, "Password" );
        if (b.Button(kk::apply, "Save & Restart")) {
          db.update();  // сохраняем БД не дожидаясь таймаута
          WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
          notice_f = true;
          //          ESP.restart();
        }
      }

      // и просто виджеты
      b.Label("lbl3"_h, "Another label", "Val", sets::Colors::Green);
      b.Label("lbl4"_h, "Привет", "Val", sets::Colors::Blue);

      b.endMenu();  // не забываем завершить меню
    }// подменю

    b.endGroup();
  }//group3

  // кнопки являются "групповым" виджетом, можно сделать несколько кнопок в одной строке
  if (b.beginButtons()) {
    // кнопка вернёт true при клике
    if (b.Button(kk::btn1, "reload")) {
      Serial.println("reload");
      b.reload();
    }

    if (b.Button(kk::btn2, "clear db", sets::Colors::Blue)) {
      Serial.println("clear db");
      db.clear();
      db.update();
    }

    b.endButtons();  // завершить кнопки
  }
}

// это апдейтер. Функция вызывается, когда вебморда запрашивает обновления
void update(sets::Updater& upd) {
  // можно отправить значение по имени (хэшу) виджета
  upd.update(kk::lbl1, random(100));
  upd.update(kk::lbl2, millis());
  if (notice_f) { // уведомление 
    notice_f = false;
    upd.notice("Теперь подключитесь к вашей wifi сети и найдите устройство");
//    upd.alert("Ошибка");
  }


  // примечание: при ручных изменениях в базе данных отправлять новые значения не нужно!
  // библиотека сделает это сама =)
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // ======== WIFI ========
  // режим AP_STA. Вызываем перед sett.begin(), чтобы
  // settings знал о текущем режиме wifi
  WiFi.mode(WIFI_AP_STA);

  /**************************** старый код от примера test
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint8_t tries = 20;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (!--tries) break;
    }
    Serial.println();
    Serial.print("Connected: ");
    Serial.println(WiFi.localIP());
    // }

    // AP
    WiFi.softAP("AP ESP");
    Serial.print("AP: ");
    Serial.println(WiFi.softAPIP());
  **************************************************/

  // ======== SETTINGS ========
  sett.begin();
  sett.onBuild(build);
  sett.onUpdate(update);

  // настройки вебморды
  // sett.config.requestTout = 3000;
  // sett.config.sliderTout = 500;
  // sett.config.updateTout = 1000;

  // ======== DATABASE ========
#ifdef ESP32
  LittleFS.begin(true);
#else
  LittleFS.begin();
#endif
  db.begin();
  db.init(kk::txt, "text");   // инициализация базы данных начальными значениями
  db.init(kk::pass, "some pass");
  db.init(kk::uintw, 64u);
  db.init(kk::intw, -10);
  db.init(kk::int64w, 1234567ll);
  db.init(kk::color, 0xff0000);
  db.init(kk::toggle, (bool)1);
  db.init(kk::slider, -3.5);
  db.init(kk::selectw, (uint8_t)1);
  db.init(kk::date, 1719941932);
  db.init(kk::timew, 60);
  db.init(kk::datime, 1719941932);
  db.init(kk::wifi_ssid, "");
  db.init(kk::wifi_pass, "");

  db.dump(Serial);

  /***************** старый
    // ======= AP =======
    WiFi.softAP("AP ESP");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    // ======= STA =======
    // если логин задан - подключаемся
    if (db[kk::wifi_ssid].length()) {
      WiFi.begin(db[kk::wifi_ssid], db[kk::wifi_pass]);
      Serial.print("Connect STA");
      int tries = 20;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
        if (!--tries) break;
      }
      Serial.println();
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
  ****************/
  // подключение и реакция на подключение или ошибку
  WiFiConnector.onConnect([]() {
    Serial.print("Connected! ");
    Serial.println(WiFi.localIP());
  });
  WiFiConnector.onError([]() {
    Serial.print("Error! start AP ");
    Serial.println(WiFi.softAPIP());
  });

  WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);

}//setup

void loop() {
  WiFiConnector.tick();
  sett.tick();  // тикер, вызывать в лупе
}
