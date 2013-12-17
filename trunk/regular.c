#include <mega32.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include "robowater.h"
#include "signals.h"
#include "regular.h"
#include "season.h"
#include "menu.h"
#include "spd1820.h"
#include "dayofweek.h"
#include "sensors.h"
#include "fan.h"
#define NODEBUG
// Описание модульных переменных
unsigned int time_integration = 0;
unsigned char timer_start_to = 0;
unsigned char timer_stop_to = 0;
unsigned int timer_start = 0;
unsigned char timer_stop = 0;
// Функция, делающая регуларный анализ значения термометров
void regular_check_alarm_and_warnings(void) {
    // Проверка свитчиков
    if (CHECK_EVENT && (!prim_par.warning_status[0]) && (SENSOR_ALARM1)) event = ev_alarm1; // Пожар, перегрев вентилятора, авария частотника
    if (CHECK_EVENT && (!prim_par.warning_status[1]) && (SENSOR_ALARM2)) event = ev_alarm2; // Угроза замораживания от внешнего датчика
    if (CHECK_EVENT && (!prim_par.warning_status[11]) && (!SENSOR_FILTER)) event = ev_filter; // Загрязнение фильтра.
    // Проверка термометров
    if (CHECK_EVENT && (!prim_par.warning_status[7]) && (termometers[0].err >= MAX_OFFLINES))
        // printf("Нет термометра В1 (Помещение): %d, err=%d", prim_par.warning_status[7], termometers[0].err);
        event = ev_term1_nf;
    if (CHECK_EVENT && (!prim_par.warning_status[8]) && (termometers[1].err >= MAX_OFFLINES))
        // Нет термометра В 2 - Улица
        event = ev_term2_nf;
    if (CHECK_EVENT && (!prim_par.warning_status[9]) && (termometers[2].err >= MAX_OFFLINES))
        // Нет термометра В 3 - Подача
        event = ev_term3_nf;
    if (CHECK_EVENT && (!prim_par.warning_status[10]) && (termometers[3].err >= MAX_OFFLINES))
        // Нет термометра В 4 - Обратка
        event = ev_term4_nf;
    // Здесь осуществляет матанализ для генерации событий
    if ((UL_T < TA_IN_NOLIMIT) && (termometers[1].err >= MAX_OFFLINES)) {
        #ifndef NODEBUG
        if (!(prim_par.warning_status[2] || prim_par.warning_status[3] || prim_par.warning_status[7] || prim_par.warning_status[10])) {
            printf ("UL_T стало меньше TA_IN_NOLIMIT\r\n");
        }
        #endif
        if (CHECK_EVENT &&
          !(prim_par.warning_status[8] || prim_par.warning_status[2]) &&
          (termometers[1].t < (prim_par.TA_in_Min - 5))) // Температура на улице ниже критической  на 5 градусов.UL_T
            event = ev_freezing1;
        if (CHECK_EVENT && !(prim_par.warning_status[7] || prim_par.warning_status[3]) &&
            (termometers[0].t < prim_par.TA_out_Min) && (termometers[1].t < prim_par.TA_out_Min)) // Температура в помещения ниже критической POM_T
               // Если тепература на улице выше критической POM_T
            event = ev_freezing2;
        if (CHECK_EVENT && !(prim_par.warning_status[10] || prim_par.warning_status[5]) &&
            (termometers[3].t < prim_par.TW_out_Min)) // Температура воды обратки ниже критической WOUT_T
            event = ev_freezing3;
    }

}
// Функция, прописывающая значения термометров по глобальным переменным 
void link_terms(void) {
    MAIN_T = read_term(0);      // Выводим информацию о главном термометре !!!
    POM_T = read_term(0);
    UL_T = read_term(1);
    WIN_T = read_term(2);
    WOUT_T = read_term(3); 
} 
// Функция, вызываемая раз в секунду по событию ev_secunda
void regular_inspection(void) {
    #ifndef NODEBUG
    // printf ("%02u:%02u Начало прерывания Секунда...", s_dt.cMM, s_dt.cSS);
    // printf (".");
    #endif
    if (timer_fan) timer_fan--;
    if (timer_start) {
        timer_start--;
        // Если достигли нуля, то включаем вентилятор
        if (!timer_start && (mode.run == mo_warming_up)) {         // event = ev_timer_Start;
             mode.fan = 1;
             #ifndef NODEBUG
             printf ("ПУСК \r\n");
             #endif
             switch_mode_run(mo_action);        // Включаем режим Пуск
             count_fan = 0;
        }
    }
    if (timer_stop) {
        timer_stop--;
        // Если достигли нуля, то выключаем вентилятор
        if (!timer_stop && (mode.run == mo_warming_down)) {          // event = ev_timer_Stop;
            switch_mode_run(mo_stop);
        }
    }
     // Пока timer1_counter > 0, уменьшаем его значение
    if (menu_timer_inactive) {
        menu_timer_inactive--;
        // Если достигли нуля, то останавливаем обслуживание valcoder
        if (!menu_timer_inactive) menu_timer_break();
    }
    if (!mode.stop_sync_dt) get_cur_dt (0);
    read_all_terms(DUTY_MODE);
    // Вычитаваем АЦП
    ADC_VAR2 = read_adc(0)/4;
    ADC_VAR1 = read_adc(1)/4;
    if (time_integration) time_integration--;
    if (time_cooling ) time_cooling-- ;
    #ifndef NODEBUG
    // printf ("конец в %02u:%02u\r\n", s_dt.cMM, s_dt.cSS);
    #endif
    s_dt.dayofweek = dayofweek(s_dt.cdd, s_dt.cmo, s_dt.cyy);
    link_terms();
    switch (mode.menu) {
        case 0: lcd_primary_screen(); break;
        // case 2: lcd_edit(0); break;
        case 1: lcd_menu(0); break;
        default: ;
    }
    // Здесь поддерживаем режимы и ведем расчеты
    switch (mode.run) {
        case mo_stop:
            time_integration = 0;
            if (prim_par.season) {
                keep_life_in_winter();
            } else {
                signal_white(OFF);
                mode.pomp = 0;
            }
            signal_green(OFF);
            time_cooling = 0;
            break;
        case mo_action:
            signal_white(prim_par.season);
            if (prim_par.season) {
                winter_regulator();
            } else {
                coolant_regulator();
            }
            // Без регулятора скорости лапа задействована для охладителя. Стоит заглушка.
            if (1 == 0) {
               // Внимание ОТКЛЮЧЕНО РЕГУЛИРОВАНИЕ СКОРОСТИ ВЕНТИЛЯТОРА!!!
                if (prim_par.season && (UL_T < TA_IN_NOLIMIT)) {
                   winter_fan_speed();
                }
            }
            break;
        case mo_setup_input1:
        case mo_setup_input2:
            if (mode.menu == 2) lcd_edit(0); 
        case mo_setup_output1:
        case mo_setup_output2:
            if (mode.menu == 0) mode.run = mode.lastrun;    // Если вышли из меню, пытаемся восстановить последний режим
            break;
        default:
            break;
    };
    // Если пора проводить ТО, то генерируем событие ev_begin_to
    if (high_time_TO()) {
        printf ("Пора проводить ТО\n");
        prim_par.TO.status = 1;
        event = ev_begin_to;
    } else {
        event = ev_none;            // Очищаем событие
        if (mode.run == mo_to) {
            if (timer_start_to) {
                timer_start_to--;
                    // Если достигли нуля, то Событие ev_to_nf
                if (!timer_start_to && (ADC_VAR1 < prim_par.ADC1_hi)) {
                    printf ("Закончилось время открытия \n");
                    event = ev_to_nf;
                }
            }
            if (timer_stop_to) {
                timer_stop_to--;
                if (ADC_VAR1 <= (prim_par.ADC1_lo + 5)) {
                   printf ("Успешное завершение ТО Крана. \n");
                   event = ev_end_to;
                } else {
                // Если достигли нуля, то Событие ev_to_nf
                    if (!timer_stop_to && (ADC_VAR1 > (prim_par.ADC1_lo + 10))) {
                        printf ("Закончилось время закрытия \n");
                        event = ev_to_nf;
                    }
                }
            }
        }
    }
}
// Функция проверяющая условия проведения ТО, и если пора - возвращает ненулевое значение
char high_time_TO(void) {
    if (prim_par.TO.weekday == s_dt.dayofweek) {
        if (prim_par.TO.hour == s_dt.cHH) {
            if (prim_par.TO.minute == s_dt.cMM) {
                if (prim_par.TO.status == 0) {
                    return (1);
                }
            }
        }
    }
    return (0);
}
// Полностью прорисовать главный экран
void lcd_primary_screen(void) {
    char run_mod [8];
    register unsigned char i;
    unsigned char c_warnings = 0;
    unsigned char sign = '+';
    int lcd_term = main_menu[0].val_data;
    #ifndef NODEBUG
    // printf ("%02u:%02u Начало прорисовки экрана...", s_dt.cMM, s_dt.cSS);
    #endif
    for (i = 0; i < MAX_WARNINGS; i++) {
        // 9-й алерт (отсутствие 3-го термометра) нам не интересен
        if (prim_par.warning_status[i] && i != 9) c_warnings++;
    }
    // Обратите внимание, что значение режима не должно быть длинее 5-ти символов
    switch (mode.run) {
        case mo_stop:           sprintf(run_mod, "СТОП "); break;
        case mo_warming_up:     sprintf(run_mod, "ПРОГР"); break;
        case mo_warming_down:   sprintf(run_mod, "ОСТАН"); break;
        case mo_action:         sprintf(run_mod, "ПУСК "); break;
        case mo_to:             sprintf(run_mod, "ТО   "); break;
        default:                sprintf(run_mod, "ИНЖЕН"); break; 
    };
    if (c_warnings)
        sprintf(run_mod, "Авария ");
    else {
        signal_red(OFF);  signal_buz(OFF); // signal_green(OFF);
    }
    lcd_command(LCD_DISP_ON);       // Убираем курсор с LCD
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 первой строки
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u %01u", s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.dayofweek);
    lcd_puts(linestr);
    lcd_gotoxy(0,1);                // Устанавливаем курсор в позицию 0 строки 2
    // Выводим информацию о термометрах
    if(ds1820_devices) {
        sign = (lcd_term < 0) ? '-' : '+';
        sprintf(linestr, "t=%c%02u.%01uC %s", sign, abs(lcd_term)/100, (abs(lcd_term)%100)/10, run_mod);
    } else {
        sprintf(linestr, "Нет термометров");
    }
    lcd_puts(linestr);
    #ifndef NODEBUG
    // printf ("конец в %02u:%02u\r\n", s_dt.cMM, s_dt.cSS);
    #endif
}
