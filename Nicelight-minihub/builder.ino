

void build(sets::Builder& b) {
  // можно узнать, было ли действие по виджету
  if (b.build.isAction()) {
    Serial.print("Set: 0x");
    Serial.print(b.build.id, HEX);
    Serial.print(" = ");
    Serial.println(b.build.value);
  }

  // группа. beginGroup всегда вернёт true для удобства организации кода
  if (b.beginGroup("Время и все такое")) {
    b.Date(kk::date, "Date");
    b.Time(kk::t1f1_startTime, "Рассвет");
    b.Time(kk::secondsNow, "Вермечко");
    //    b.Time("", &data.secondsNow);// так было


    b.DateTime(kk::datime, "Datime");
    b.endGroup();  // НЕ ЗАБЫВАЕМ ЗАВЕРШИТЬ ГРУППУ
  }

  // пара лейблов вне группы. Так тоже можно
  b.Label(kk::lbl1, "lable1");
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

    // а это кнопка на вложенное меню. Далее нужно описать его содержимое
    if (b.beginMenu("Подменю")) {
      b.Input(kk::txt, "Text");
      b.Pass(kk::pass, "Password");
      b.Input(kk::uintw, "uint");
      b.Input(kk::intw, "int");
      b.Input(kk::int64w, "int 64");
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

      ////////////////////
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
        /////////////////////
        b.endButtons();  // завершить кнопки
      }
      b.endMenu();  // не забываем завершить меню
    }// подменю

    b.endGroup();
  }//group3

}//builder


// это апдейтер. Функция вызывается, когда вебморда запрашивает обновления
void update(sets::Updater& upd) {
  // можно отправить значение по имени (хэшу) виджета
  upd.update(kk::lbl1, random(100));
  upd.update(kk::lbl2, millis());
  upd.update(kk::secondsNow, data.secondsNow);
  if (notice_f) { // уведомление
    notice_f = false;
    upd.notice("Теперь подключитесь к вашей wifi сети и найдите устройство");
    //    upd.alert("Ошибка");
  }
  // примечание: при ручных изменениях в базе данных отправлять новые значения не нужно!
  // библиотека сделает это сама =)
}//update
