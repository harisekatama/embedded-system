#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <DS3231.h>
#include <PS2Keyboard.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  4

#define MX_CLK_PIN   13  // DOT MATRIX SCK
#define MX_DATA_PIN  11  // DOT MATRIX MOSI
#define MX_CS_PIN    10  // DOT MATRIX SS
#define KB_DATA_PIN   3  // KEYBOARD DATA
#define KB_IRQ_PIN    2  // KEYBOARD IRQ

MD_Parola mydisplay(HARDWARE_TYPE, MX_CS_PIN, MAX_DEVICES);
char buf[99] = {""};

DS3231 myrtc(SDA, SCL);
PS2Keyboard mykeyboard;

Time timezone;
String hours;
String minutes;
float ldr;
float temperature;

String temp_alarm_hours;
String temp_alarm_minutes;
int temp_alarm_duration = 5;
bool temp_alarm_active;
int selected_alarm;
String alarm_5_msg;

// 1 -> jam
// 2 -> pilihan menu
// 3 -> setting jam
// 4 -> pilihan 5 alarm
// 5 -> setting alarm aktif atau tidak
// 6 -> seting waktu alarm
// 7 -> setting durasi
// 8 -> alarm aktif
// 9 -> input custom text
int state = 1;

// 1 -> set jam
// 2 -> set alarm
int current_menu = 1;

// list alarm 1 - 5
int current_alarm_list_menu = 1;

// on off menu. 1 -> on, 2 -> off
int current_alarm_on_off_menu = 1;

// alarm yg sedang bunyi, 1 - 5
int current_beeping_alarm = 0;

bool is_setting_hour = false;

struct alarm {
  bool active;
  int hours;
  int minutes;
  int duration;
};

alarm alarm1 = {false, 0, 0, 0};
alarm alarm2 = {false, 0, 0, 0};
alarm alarm3 = {false, 0, 0, 0};
alarm alarm4 = {false, 0, 0, 0};
alarm alarm5 = {false, 0, 0, 0};
alarm alarms[5] = {alarm1, alarm2, alarm3, alarm4, alarm5};

unsigned long cur_time;
unsigned long prev_time;

void setup()
{
  Serial.begin(9600);
  myrtc.begin();
  mydisplay.begin();
  mydisplay.displayClear();
  mydisplay.setCharSpacing(1);
  mydisplay.displayText(buf, PA_CENTER, 100, 0, PA_PRINT, PA_NO_EFFECT);
  attachInterrupt(digitalPinToInterrupt(KB_DATA_PIN), interruptHandler, FALLING);
  mykeyboard.begin(KB_DATA_PIN, KB_IRQ_PIN, PS2Keymap_US);
}

void loop()
{
   if (alarms[0].active) {
      Serial.println("Alarm 1 ON");
   }
   if (alarms[1].active) {
      Serial.println("Alarm 2 ON");
   }
   if (alarms[2].active) {
      Serial.println("Alarm 3 ON");
   }
   if (alarms[3].active) {
      Serial.println("Alarm 4 ON");
   }
   if (alarms[4].active) {
      Serial.println("Alarm 5 ON");
   }
  switch (state) {
    case 1:
      timezone = myrtc.getTime();
  
      temperature = ((float) analogRead(A1) / 1023) * 500 - 10;
      
      // cek alarm
      for (int i = 0; i < 5; i++) {
        if (alarms[i].hours == timezone.hour && alarms[i].minutes == timezone.min && alarms[i].active) {
          state = 8;
          mydisplay.displayReset();
          mydisplay.displayClear();
          current_beeping_alarm = i + 1;
          prev_time = millis();
        }
      }
      // Keluar loop kalau alarm bunyi
      if (state == 8) {
        break;
      }
      
      ldr = analogRead(A0);
      mydisplay.setIntensity((ldr/500)*15);
      
      if (String(timezone.hour).length() > 1) {
        hours = String(timezone.hour);
      } else {
        hours = "0" + String(timezone.hour);
      }
      if (String(timezone.min).length() > 1) {
        minutes = String(timezone.min);
      } else {
        minutes = "0" + String(timezone.min);
      }
      if (timezone.sec == 40 || timezone.sec == 10) {
        String(temperature).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        
        delay(5000);
      } else {
        (hours + " " + minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        (hours + ":" + minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
      }
      break;
    case 2:
      switch (current_menu) {
        case 1:
          String("Jam").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
        case 2:
          String("Alarm").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
      }
      break;
    case 3:
      if (hours.length() == 1) {
        hours = "0" + hours;
      }
      if (minutes.length() == 1) {
        minutes = "0" + minutes;
      }
      if (is_setting_hour) {
        (hours + ":" + minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        ("   :" + minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        break;
      } else {
        (hours + ":" + minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        (hours + ":   ").toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        break;
      }
    case 4:
      switch (current_alarm_list_menu) {
        case 1:
          String("1").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
        case 2:
          String("2").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
        case 3:
          String("3").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
        case 4:
          String("4").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
        case 5:
          String("5").toCharArray(buf, 99);
          mydisplay.displayReset();
          mydisplay.setTextBuffer(buf);
          mydisplay.displayAnimate();
          break;
      }
      break;
    case 5:
      if (current_alarm_on_off_menu == 1) {
        String("On").toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
      } else {
        String("Off").toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
      }
      break;
    case 6:
      if (temp_alarm_hours.length() == 1) {
        temp_alarm_hours = "0" + temp_alarm_hours;
      }
      if (temp_alarm_minutes.length() == 1) {
        temp_alarm_minutes = "0" + temp_alarm_minutes;
      }
      if (is_setting_hour) {
        (temp_alarm_hours + ":" + temp_alarm_minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        ("   :" + temp_alarm_minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        break;
      } else {
        (temp_alarm_hours + ":" + temp_alarm_minutes).toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        (temp_alarm_hours + ":   ").toCharArray(buf, 99);
        mydisplay.displayReset();
        mydisplay.setTextBuffer(buf);
        mydisplay.displayAnimate();
        delay(500);
        break;
      }
      break;
    case 7:
      (String(temp_alarm_duration) + " s").toCharArray(buf, 99);
      mydisplay.displayReset();
      mydisplay.setTextBuffer(buf);
      mydisplay.displayAnimate();
      break;
    case 8:
      if (millis() - prev_time > (alarms[current_beeping_alarm - 1].duration * 1000)) {
        alarms[current_beeping_alarm - 1].active = false;
        state = 1;
      } else {
        Serial.println(current_beeping_alarm);
        switch (current_beeping_alarm) {
          case 1:
            Serial.println("alarm 1");
            mydisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
            String("07211940000046").toCharArray(buf, 99);
            mydisplay.setTextBuffer(buf);
            if (mydisplay.displayAnimate()) {
              mydisplay.displayReset();
              mydisplay.displayClear();
            }
            break;
          case 2:
            Serial.println("alarm 2");
            mydisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
            String("I Putu Haris Setiadi").toCharArray(buf, 99);
            mydisplay.setTextBuffer(buf);
            if (mydisplay.displayAnimate()) {
              mydisplay.displayReset();
              mydisplay.displayClear();
            }
            break;
          case 3:
            Serial.println("alarm 3");
            mydisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
            String("I Putu Haris Setiadi | 07211940000046").toCharArray(buf, 99);
            mydisplay.setTextBuffer(buf);
            if (mydisplay.displayAnimate()) {
              mydisplay.displayReset();
              mydisplay.displayClear();
            }
            break;
          case 4:
            Serial.println("alarm 4");
            mydisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
            (String(alarms[current_beeping_alarm - 1].duration) + " s").toCharArray(buf, 99);
            mydisplay.setTextBuffer(buf);
            if (mydisplay.displayAnimate()) {
              mydisplay.displayReset();
              mydisplay.displayClear();
            }
            break;
          case 5:
            Serial.println("alarm 5");
            mydisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
            alarm_5_msg.toCharArray(buf, 99);
            mydisplay.setTextBuffer(buf);
            if (mydisplay.displayAnimate()) {
              mydisplay.displayReset();
              mydisplay.displayClear();
            }
            break;
        } 
      }
      break;
    case 9:
      
      mydisplay.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
      
      alarm_5_msg.toCharArray(buf, 99);
      mydisplay.setTextBuffer(buf);
      
      if (mydisplay.displayAnimate()) {
        mydisplay.displayReset();
        mydisplay.displayClear();
      }
      break;
  }
  mydisplay.setTextEffect(PA_PRINT, PA_NO_EFFECT);
}

void interruptHandler() {
  switch (state) {
    case 1:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_ENTER) {
          state = 2;
        }
      }
      break;
    case 2:
      switch (current_menu) {
        case 1:
          if (mykeyboard.available()) {
            char c = mykeyboard.read();
            if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
              current_menu = 2;
            } else if (c == PS2_ENTER) {
              state = 3;
              is_setting_hour = true;
            } else if (c == PS2_ESC) {
              state = 1;
            }
          }
        case 2:
          if (mykeyboard.available()) {
            char c = mykeyboard.read();
            if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
              current_menu = 1;
            } else if (c == PS2_ENTER) {
              state = 4;
            } else if (c == PS2_ESC) {
              state = 1;
              current_menu = 1;
            }
          }
          break;
      }
      break;
    case 3:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_ESC) {
          state = 2;
        } else if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
          is_setting_hour = !is_setting_hour;
        } else if (c == PS2_UPARROW) {
          if (is_setting_hour) {
            if (hours.toInt() < 23 && hours.toInt() >= 0) {
              hours = String(hours.toInt() + 1);
            } else if (hours.toInt() == 23) {
              hours = String(0);
            }
          } else {
            if (minutes.toInt() < 59 && minutes.toInt() >= 0) {
              minutes = String(minutes.toInt() + 1);
            } else if (minutes.toInt() == 59) {
              minutes = String(0);
            }
          }
        } else if (c == PS2_DOWNARROW) {
          if (is_setting_hour) {
            if (hours.toInt() <= 23 && hours.toInt() > 0) {
              hours = String(hours.toInt() - 1);
            } else if (hours.toInt() == 0) {
              hours = String(23);
            }
          } else {
            if (minutes.toInt() <= 59 && minutes.toInt() > 0) {
              minutes = String(minutes.toInt() - 1);
            } else if (minutes.toInt() == 59) {
              minutes = String(59);
            }
          }
        } else if (c == PS2_ENTER) {
          myrtc.setTime(hours.toInt(), minutes.toInt(), 00);
          state = 1;
          is_setting_hour = true;
        }
      }
      break;
    case 4:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_LEFTARROW) {
          if (current_alarm_list_menu == 1) {
            current_alarm_list_menu = 5;
          } else {
            current_alarm_list_menu -= 1;
          }
        } else if (c == PS2_RIGHTARROW) {
          if (current_alarm_list_menu == 5) {
            current_alarm_list_menu = 1;
          } else {
            current_alarm_list_menu += 1;
          }
        } else if (c == PS2_ENTER) {
          if (alarms[current_alarm_list_menu - 1].active) {
            state = 5;
            temp_alarm_hours = alarms[current_alarm_list_menu - 1].hours;
            temp_alarm_minutes = alarms[current_alarm_list_menu - 1].minutes;
          } else {
            temp_alarm_hours = hours;
            temp_alarm_minutes = minutes;
            state = 6;
          }
          selected_alarm = current_alarm_list_menu;
          
        } else if (c == PS2_ESC) {
          state = 3;
          current_alarm_list_menu = 1;
        }
      }
      break;
    case 5:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
          if (current_alarm_on_off_menu == 1) {
            current_alarm_on_off_menu = 2;
          } else {
            current_alarm_on_off_menu = 1;
          }
        } else if (c == PS2_ENTER) {
          if (current_alarm_on_off_menu == 1) {
            state = 6;
          } else {
            state = 1;
            current_alarm_on_off_menu = 1;
            alarms[selected_alarm - 1].active = false;
          }
        }
      }
    case 6:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_ESC) {
          if (alarms[selected_alarm - 1].active) {
            state = 5;
          } else {
            state = 4;
          }
        } else if (c == PS2_LEFTARROW || c == PS2_RIGHTARROW) {
          is_setting_hour = !is_setting_hour;
        } else if (c == PS2_UPARROW) {
          if (is_setting_hour) {
            if (temp_alarm_hours.toInt() < 23 && temp_alarm_hours.toInt() >= 0) {
              temp_alarm_hours = String(temp_alarm_hours.toInt() + 1);
            } else if (temp_alarm_hours.toInt() == 23) {
              temp_alarm_hours = String(0);
            }
          } else {
            if (temp_alarm_minutes.toInt() < 59 && temp_alarm_minutes.toInt() >= 0) {
              temp_alarm_minutes = String(temp_alarm_minutes.toInt() + 1);
            } else if (temp_alarm_minutes.toInt() == 59) {
              temp_alarm_minutes = String(0);
            }
          }
        } else if (c == PS2_DOWNARROW) {
          if (is_setting_hour) {
            if (temp_alarm_hours.toInt() <= 23 && temp_alarm_hours.toInt() > 0) {
              temp_alarm_hours = String(temp_alarm_hours.toInt() - 1);
            } else if (temp_alarm_hours.toInt() == 0) {
              temp_alarm_hours = String(23);
            }
          } else {
            if (temp_alarm_minutes.toInt() <= 59 && temp_alarm_minutes.toInt() > 0) {
              temp_alarm_minutes = String(temp_alarm_minutes.toInt() - 1);
            } else if (temp_alarm_minutes.toInt() == 59) {
              temp_alarm_minutes = String(59);
            }
          }
        } else if (c == PS2_ENTER) {
          state = 7;
          is_setting_hour = true;
        }
      }
      break;
    case 7:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_UPARROW) {
          temp_alarm_duration += 1;
        } else if (c == PS2_DOWNARROW && temp_alarm_duration > 1) {
          temp_alarm_duration -= 1; 
        } else if (c == PS2_ESC) {
          state = 6;
        } else if (c == PS2_ENTER) {
          if (selected_alarm == 5) {
            state = 9;
          } else {
            state = 1;
            alarms[selected_alarm - 1].hours = temp_alarm_hours.toInt();
            alarms[selected_alarm - 1].minutes = temp_alarm_minutes.toInt();
            alarms[selected_alarm - 1].duration = temp_alarm_duration;
            alarms[selected_alarm - 1].active = true;
          }   
        }
      }
      break;
    case 8:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_ENTER) {
          state = 1;
          alarms[current_beeping_alarm - 1].active = false;
        }
      }
      break;
    case 9:
      if (mykeyboard.available()) {
        char c = mykeyboard.read();
        if (c == PS2_ENTER) {
          alarms[selected_alarm - 1].hours = temp_alarm_hours.toInt();
          alarms[selected_alarm - 1].minutes = temp_alarm_minutes.toInt();
          alarms[selected_alarm - 1].duration = temp_alarm_duration;
          alarms[selected_alarm - 1].active = true;
          state = 1;
        } else {
          alarm_5_msg = alarm_5_msg + String(c);
          mydisplay.displayReset();
          mydisplay.displayClear();
        }
      }
      break;
  }
}
