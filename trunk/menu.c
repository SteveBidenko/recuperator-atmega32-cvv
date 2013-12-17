#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ds1307.h> // DS1307 Real Time Clock functions
#include <delay.h>
#include "robowater.h"
#include "fan.h"
#include "menu.h"
#include "spd1820.h"
#include "alarm.h"
#include "at2404.h"
#include "regular.h"
// Объявление макроподстановок
// Структура основных переменных
// signed char curr_menu_level1 = 0, next_menu_level1 = 1;  // Текущий и следующий пункт меню
struct st_array_pos curr_menu;                // Позиции в меню
struct st_array_pos dt_curr_menu;             // Здесь сохраняем главное меню во время входа в меню установки даты и времени
unsigned char menu_timer_inactive;          // Таймер бездействия меню, по срабатыванию которого выходим из него
struct st_parameter main_menu[NUM_MENU]= {   // Меню первого уровня
    {-9900, e_temperature, 0, 1},     // [0]
    {0, e_mode, 1, 2},                // [1] Задание текущего режима
    {1, e_winter, 0, 3},              // [2] ЗИМА
    {0, e_dt, 1, 4},                  // [3] Установка даты и времени
    {0, e_empty, 1, 5},               // [4] Вход в меню просмотра параметров
    {0, e_alarm, 0, 6},               // [5] АВАРИЙ НЕТ (при отладке программы установить {0, e_alarm, 1, 6})
    {0, e_empty, 0, 7}                // [6] Вход в меню тонких настроек (при отладке программы установить {0, e_empty, 1, 7})
};

struct st_parameter sdt[NUM_DT]= {   // Меню Установки времени и даты
    {20, e_hour, 1, 8},             // [0]
    {05, e_minute, 1, 9},           // [1]
    {19, e_day, 1, 10},             // [2]
    {03, e_month, 1, 11},           // [3]
    {11, e_year, 1, 12},            // [4]
    {0, e_winter, 1, 33},           // [5] ЗИМА
    {0, e_password, 1, 34}          // [6]
};
// Рабочие параметры
struct st_parameter parameters[NUM_PARAMETERS]= {
    {2200, e_room, 1, 15},          // [0] Заданная температура воздуха TA_out_prs
    {2000, e_temperature, 0, 13},   // [1] Температура воздуха в помещении
    {-350, e_temperature, 0, 14},   // [2] Температура воздуха на улице
    {7200, e_temperature, 0, 16},   // [3] Температура воды на входе
    {6500, e_temperature, 0, 17},   // [4] Температура воды на выходе
    {40, e_percent, 0, 18},         // [5] Текущее состояние крана (АЦП)
    {42, e_percent, 1, 19},         // [6] Заданное состояние крана (расчетное PWM)
    {95, e_percent, 0, 20},         // [7] Текущее состояние вентилятора (АЦП)
    {100, e_percent, 1, 21},        // [8] Заданное состояние вентилятора (расчетное PWM)
    {1, e_clatsman, 0, 22},         // [9] Положение воздушной заслонки (0 - открыта, 1 - закрыта)
    {1, e_clatsman, 1, 23}          // [10] Работа насоса (1 - вкл., 0 - выкл.)
};
// Инженерная наладка (тонкие настройки)
struct st_parameter settings[NUM_SETTINGS]={
    {-2500, e_world, 1, 24},        // [0] температура воздуха мин TA_in_Min
    {3500, e_water, 1, 25},         // [1] температура воды СТОП TW_out_Stop
    {1000, e_water, 1, 26},         // [2] температура воды выход TW_out_Min
    {1500, e_room, 1, 27},          // [3] температура воздуха мин TA_out_Min
    {30, e_stime, 1, 28},           // [4] время интегрирования лето T_z ?
    {10, e_stime, 1, 29},           // [5] время интегрирования зима T_int
    {1, e_coef, 1, 30},             // [6] Коэффициэнт усиления Ku
    {0, e_coef, 1, 54},             // [7] Коэффициэнт Ki
    {0, e_coef, 1, 55},             // [8] Коэффициэнт Kd
    {0, e_delete, 1, 35},           // [9] Отключение термометра 1
    {-1, e_address, 1, 36},         // [10] установка термометра 1
    {1, e_delete, 1, 37},           // [11] Отключение термометра 2
    {-1, e_address, 1, 38},         // [12] установка термометра 2
    {2, e_delete, 1, 39},           // [13] Отключение термометра 3
    {-1, e_address, 1, 40},         // [14] установка термометра 3
    {3, e_delete, 1, 41},           // [15] Отключение термометра 4
    {-1, e_address, 1, 42},         // [16] установка термометра 4
    {48, e_PWM1, 1, 43},             // [17] Установка нижней границы PWM1
    {250, e_PWM1, 1, 44},            // [18] Установка верхней границы PWM1
    {48, e_ADC1, 1, 45},             // [19] Установка нижней границы ADC1
    {250, e_ADC1, 1, 46},            // [20] Установка верхней границы ADC1
    {48, e_PWM2, 1, 47},             // [21] Установка нижней границы PWM2
    {250, e_PWM2, 1, 48},            // [22] Установка верхней границы PWM2
    {48, e_ADC2, 1, 49},             // [23] Установка нижней границы ADC2
    {250, e_ADC2, 1, 50},           // [24] Установка верхней границы ADC2
    {1, e_weekday, 1, 51},          // [25] Установка дня недели
    {0, e_hour, 1, 52},             // [26] Установка часа проведения ТО Крана
    {0, e_minute, 1, 53},           // [27] Установка минуты проведения ТО Крана
    {0, e_temperature, 1, 31},      // [28] Дельта лето dt_summer;
    {0, e_temperature, 1, 32},      // [29] Дельта зима dt_winter;
    {180, e_stime, 1, 56}           // [30] Время прогрева при старте системы  
};
#define WARNING_POS 61
struct st_parameter warnings[MAX_WARNINGS] = {
    {0, e_warning, 0, WARNING_POS},
    {0, e_warning, 0, WARNING_POS + 1},
    {0, e_warning, 0, WARNING_POS + 2},
    {0, e_warning, 0, WARNING_POS + 3},
    {0, e_warning, 0, WARNING_POS + 4},
    {0, e_warning, 0, WARNING_POS + 5},
    {0, e_warning, 0, WARNING_POS + 6},
    {0, e_warning, 0, WARNING_POS + 7},
    {0, e_warning, 0, WARNING_POS + 8},
    {0, e_warning, 0, WARNING_POS + 9},
    {0, e_warning, 0, WARNING_POS + 10},
    {0, e_warning, 0, WARNING_POS + 11}
};
flash lcd_str all_menu_str[] = {
        "Помещение ",  // [0]
        "РЕЖИМ ",      // [1]
        "СЕЗОН ",      // [2]
        "Уст. ",       // [3]
        "Параметры ",  // [4]
        "АВАРИЙ ",      // [5]
        "НАСТРОЙКА...", // [6]
        "Уст.Час. ",   // [7]
        "Уст Мин ",    // [8]
        "Уст.день. ",  // [9]
        "Уст Мес. ",   // [10]
        "Уст ГОД. ",   // [11]
        "ПОМЕЩ. ",     // [12]
        "УЛ.Т ",       // [13]
        "Уст.Т ",      // [14] TA_out_prs
        "Вод ВХ. ",    // [15]
        "Вод ВЫХ.",    // [16]
        "КРАН ",       // [17]
        "УСТ.КРАН. ",  // [18]
        "ВЕНТИЛЯТОР ", // [19]
        "Уст. ВЕНТ. ", // [20]
        "ЗАСЛ. ",      // [21]
        "НАСОС ",      // [22]
        "TA IN Min ",  // [23] TA_in_Min
        "TW STOP ",    // [24] TW_out_Stop
        "TW Min ",     // [25] TW_out_Min
        "TA Min ",     // [26] TA_out_Min
        "ВР.Лето=",    // [27] T_z
        "ВР.Зима=",    // [28] T_int
        "КУ=",         // [29] 
        "Дt Лето=",    // [30] t_summer_sensitivity;
        "Дt Зима=",     // [31] t_winter_sensitivity;
        "Сезон=",       // [32]
        "Пароль=",      // [33]
        "DEL TE1 Пом.",     // [34] Важен порядок следования, т.е. с 34 - по 41 должны идти 4 термомтера подряд
        "SET TE1 Пом.",     // [35]
        "DEL TE2 Ул.",      // [36]
        "SET TE2 Ул.",      // [37]
        "DEL TE3 WIn",      // [38]
        "SET TE3 WIn",      // [39]
        "DEL TE4 WOut",     // [40]
        "SET TE4 WOut",     // [41]
        "Вых1.сн.=",    // [42] Установление вых1 напряжение снизу
        "Вых1.св.=",    // [43] Установление вых1 напряжение сверху
        "Вх1.сн.=",     // [44] Установление вх1 напряжение снизу
        "Вх1.св.=",     // [45] Установление вх1 напряжение сверху
        "Вых2.сн.=",    // [46] Установление вых2 напряжение снизу
        "Вых2.св.=",    // [47] Установление вых2 напряжение сверху
        "Вх2.сн.= ",    // [48] Установление вх2 напряжение снизу
        "Вх2.св.=",     // [49] Установление вх2 напряжение сверху
        "День ТО=",     // [50] День недели ТО
        "Час ТО=",      // [51] Установка часа проведения ТО Крана
        "Минут ТО=",    // [52] Установка минуты проведения ТО Крана
        "Ki=",          // [53]
        "Kd=",          // [54]
        "Прогрев="      // [55]
};
char linestr[20];           // Строка для LCD
bit need_eeprom_write;      // Флаг, если необходимо записать в EEPROM
int menu_value;             // текущее значение настраиваемого параметра
// Функция синхронизации структуры основных переменных
void sync_set_par(byte sync) {
    register byte i, j;
    // Если входим в меню
    if (sync == SYNC_TO_MENU) {
        main_menu[2].val_data = prim_par.season;
        settings[0].val_data = prim_par.TA_in_Min;
        settings[1].val_data = prim_par.TW_out_Stop;
        settings[2].val_data = prim_par.TW_out_Min;
        settings[3].val_data = prim_par.TA_out_Min;
        settings[4].val_data = prim_par.T_z;
        settings[5].val_data = prim_par.T_int;
        settings[6].val_data = prim_par.Ku;
        settings[7].val_data = prim_par.Ki;
        settings[8].val_data = prim_par.Kd;
        // перенес из boiler-control 15.05.2013
        settings[17].val_data = PWM_MIN;
        settings[18].val_data = PWM_MAX;
        settings[19].val_data = prim_par.ADC1_lo;
        settings[20].val_data = prim_par.ADC1_hi;
        settings[21].val_data = prim_par.PWM2_lo;
        settings[22].val_data = prim_par.PWM2_hi;
        settings[23].val_data = prim_par.ADC2_lo;
        settings[24].val_data = prim_par.ADC2_hi;
        // ТО крана
        settings[25].val_data = prim_par.TO.weekday;
        settings[26].val_data = prim_par.TO.hour;
        settings[27].val_data = prim_par.TO.minute;
        settings[28].val_data = prim_par.dt_summer;
        settings[29].val_data = prim_par.dt_winter;
        settings[30].val_data = prim_par.T_start;

        // Здесь будет разрешение/запрет на редактирование в меню
        for (i = 0; i < MAX_DS1820; i++) {
            unsigned char i_set9 = 9 + (int)i * 2;
            unsigned char i_set10 = 10 + (int)i * 2;

            // Если есть ошибки на термометре и есть новые термометры, то разрешаем установку
            if (termometers[i].err) {
                // Выключаем DEL и включаем SET
                settings[i_set9].can_edit = 0;
				// Если нет термометров, вообще ничего не разрешаем
                settings[i_set10].can_edit = (mode.new_terms) ? 1 : 0;
            } else {
                // Если нет термометров, вообще ничего не разрешаем
                // settings[i_set9].can_edit = (mode.new_terms) ? 1 : 0;
                // Вариант 2: разрешаем абсолютное удаление из системы
                settings[i_set9].can_edit = 1;
                settings[i_set10].can_edit = 0;
            }
            settings[i_set9].val_data = i;
            settings[i_set10].val_data = -1;
        }
        if (mode.run == mo_stop) {  // Запоминает только в режиме останова
            FAN_SPEED = prim_par.fan_speed;
        };
        parameters[0].val_data = prim_par.TA_out_prs;
        parameters[10].val_data = mode.pomp;
        // printf("Чтени времени и даты \r\n");
        sdt[0].val_data = s_dt.cHH;
        sdt[1].val_data = s_dt.cMM;
        sdt[2].val_data = s_dt.cdd;
        sdt[3].val_data = s_dt.cmo;
        sdt[4].val_data = s_dt.cyy;
        SEASON = prim_par.season;
    } else {
        // Если выходим из меню
        if (mode.stop_sync_dt) {
            if ((s_dt.cHH != sdt[0].val_data) || (s_dt.cMM != sdt[1].val_data)) {
                s_dt.cHH = sdt[0].val_data;
                s_dt.cMM = sdt[1].val_data;
                s_dt.cSS = 0;
                set_cur_dt();
                printf("Запись времени %02u:%02u:00\r\n",sdt[0].val_data, sdt[1].val_data);
                printf("Дата %02u.%02u.20%02u\r\n", sdt[2].val_data, sdt[3].val_data, sdt[4].val_data);
            };
            if ((s_dt.cdd != sdt[2].val_data) || (s_dt.cmo != sdt[3].val_data) || (s_dt.cyy != sdt[4].val_data)) {
                s_dt.cdd = sdt[2].val_data;
                s_dt.cmo = sdt[3].val_data;
                s_dt.cyy = sdt[4].val_data;
                set_cur_dt();
                printf("Запись даты %02u.%02u.20%02u\r\n", sdt[2].val_data, sdt[3].val_data, sdt[4].val_data);
                printf("Время %02u:%02u:00\r\n",sdt[0].val_data, sdt[1].val_data);
            };
        } else {
            // Проверяем каждый параметр и если он отличается от начального, то выставляем флаг необходимости записи в EEPROM
            if (prim_par.TA_in_Min != settings[0].val_data) {
                prim_par.TA_in_Min = settings[0].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TW_out_Stop != settings[1].val_data) {
                prim_par.TW_out_Stop = settings[1].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TW_out_Min != settings[2].val_data) {
                prim_par.TW_out_Min = settings[2].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TA_out_Min != settings[3].val_data) {
                prim_par.TA_out_Min = settings[3].val_data; need_eeprom_write = 1;
            }
            if (prim_par.T_z != settings[4].val_data) {
                prim_par.T_z = settings[4].val_data; need_eeprom_write = 1;
            }
            if (prim_par.T_int != settings[5].val_data) {
                prim_par.T_int = settings[5].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Ku != settings[6].val_data) {
                prim_par.Ku = settings[6].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Ki != settings[7].val_data) {
                prim_par.Ki = settings[7].val_data; need_eeprom_write = 1;
            }
            if (prim_par.Kd != settings[8].val_data) {
                prim_par.Kd = settings[8].val_data; need_eeprom_write = 1;
            }
            // перенес из boiler-control 15.05.2013
            if (PWM_MIN != settings[17].val_data) {
                PWM_MIN = settings[17].val_data; need_eeprom_write = 1;
            }
            if (PWM_MAX != settings[18].val_data) {
                PWM_MAX = settings[18].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC1_lo != settings[19].val_data) {
                prim_par.ADC1_lo = settings[19].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC1_hi != settings[20].val_data) {
                prim_par.ADC1_hi = settings[20].val_data; need_eeprom_write = 1;
            }
            if (prim_par.PWM2_lo != settings[21].val_data) {
                prim_par.PWM2_lo = settings[21].val_data; need_eeprom_write = 1;
            }
            if (prim_par.PWM2_hi != settings[22].val_data) {
                prim_par.PWM2_hi = settings[22].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC2_lo != settings[23].val_data) {
                prim_par.ADC2_lo = settings[23].val_data; need_eeprom_write = 1;
            }
            if (prim_par.ADC2_hi != settings[24].val_data) {
                prim_par.ADC2_hi = settings[24].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.weekday != settings[25].val_data) {
                prim_par.TO.weekday = settings[25].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.hour != settings[26].val_data) {
                prim_par.TO.hour = settings[26].val_data; need_eeprom_write = 1;
            }
            if (prim_par.TO.minute != settings[27].val_data) {
                prim_par.TO.minute = settings[27].val_data; need_eeprom_write = 1;
            }
            if (prim_par.dt_summer != settings[28].val_data) {
                prim_par.dt_summer = settings[28].val_data; need_eeprom_write = 1;
            }
            if (prim_par.dt_winter != settings[29].val_data) { 
                prim_par.dt_winter = settings[29].val_data; need_eeprom_write = 1;
            }
            if (prim_par.T_start != settings[30].val_data) { 
                prim_par.T_start = settings[30].val_data; need_eeprom_write = 1;
            }
            // Проверяем часть меню, где идет управление термометрами
            for (i = 0; i < MAX_DS1820; i++) {
                unsigned char i_set9 = 9 + (int)i * 2;
                unsigned char i_set10 = 10 + (int)i * 2;
                int val_data_set_9 = settings[i_set9].val_data,
                    val_data_set_10 = settings[i_set10].val_data;

                // проверяем на отрицательность DEL
                if (val_data_set_9 < 0) {
                    // Переносим адрес в ufo для последующей привязки, если он есть на линии физически
                    if (termometers[i].err == 0) {
                        for (j = 0; j < 9; j++) {
                             mode.ufo[mode.new_terms][j] = ds1820_rom_codes[i][j];
                        }
                        mode.new_terms++;
                    }
                    // Затираем адрес в prim_par и ds1820_rom_codes
                    ds1820_rom_codes[i][1] = ds1820_rom_codes[i][0] = prim_par.addr[i][0] = 0xFF;
                    // Запрещаем второй раз входить в это меню
                    settings[i_set9].can_edit = 0;
                    if (mode.new_terms) settings[i_set10].can_edit = 1;
                    need_eeprom_write = 1;
                    // Для того, чтобы второй раз не входил в этот блок
                    settings[i_set9].val_data = i;
                }
                // Прверяем положительность SET
                if (val_data_set_10 >= 0) {
                    // printf("Перешиваем номер в ufo: settings[i_set10].val_data = %d\r\n", val_data_set_10);
                    for (j = 0; j < 9; j++) {
                        // Переписываем из неизвестных mode.ufo в prim_par.addr и ds1820_rom_codes
                        ds1820_rom_codes[i][j] = prim_par.addr[i][j] = mode.ufo[val_data_set_10][j];
                        // Перемещаем адрес термометра ближе к началу матрицы ufo
                        mode.ufo[val_data_set_10][j] = mode.ufo[mode.new_terms - 1][j];
                    }
                    mode.new_terms--;
                    need_eeprom_write = 1;
                    settings[i_set9].can_edit = 1;
                    settings[i_set10].can_edit = 0;
                    // Для того, чтобы второй раз не входил в этот блок
                    settings[i_set10].val_data = -1;
                }
            }
            if (prim_par.TA_out_prs != parameters[0].val_data) {
                prim_par.TA_out_prs = parameters[0].val_data; need_eeprom_write = 1;
            };
            if (mode.run == mo_stop) {
                if (prim_par.fan_speed != FAN_SPEED) {
                    prim_par.fan_speed = FAN_SPEED = check_fan_range(FAN_SPEED); 
                    need_eeprom_write = 1;
                };
            };
            if (mode.pomp  != parameters[10].val_data) {
                mode.pomp = parameters[10].val_data;
            };
            // Здесь применяется использование режимов mo_* с привязкой к месту.
            if ((int)mode.run != MAIN_MODE) {
                // В режиме mode.initrun прописываем значение такое, чтобы оно выходило за пределы диапазона режимов с тем, чтобы в check_peripheral() сгенерировать нужное событие
                mode.initrun = (unsigned char)main_menu[1].val_data + INITMODE;
            };
            for (i = 0; i < MAX_WARNINGS; i++) {
                if (prim_par.warning_status[i] && (warnings[i].val_data == 0)) {
                 alarm_unreg (i);
                 printf ("Удалили активную тревогу: %s\r\n", get_warning_str(i));
                 read_all_terms(INIT_MODE);
                 printf ("Инициализируем все термометры!\r\n");
                 }
            }
        };
        // Выставление сезона находится в меню установки часов. Необходимо принудительно записивать в eeprom
        if (prim_par.season != SEASON) {
            prim_par.season = SEASON; need_eeprom_write = 1;
            SEASON_MAIN = SEASON;
        }
     };
}
// Функция, которая возвращает строку со строковым названием параметра, и его числовым значением
// Аргументы - номер элемента массива и указатель на массив
// Возврат - указатель на строку linestr
char *param_str(byte num_line, struct st_parameter parameter[]) {
    int pr_data;
    struct st_parameter *st_pointer;
    st_pointer = &parameter[num_line];          // Передаем конкретный указатель на структуру текущего элемента массива
    pr_data = st_pointer->val_data;
    /* struct st_parameter *pnt; pnt = parameter + num_line; */
    return (par_str(st_pointer, 0, pr_data));
}
// Универсальная функция возвращающая строку с преобразованным типовым значением.
// 1) Аргумент - указатель на структуру параметра
// 2) Флаг - признак вывода только значения. (0 - выводить строковое название, 1 - не выводить строковое название)
// 3) Данные, которые нужно подставить
// Возврат - указатель на строку linestr
char *par_str(struct st_parameter *st_pointer, unsigned char only_val, int pr_data) {
    char *pr_name;
    char prompt[] = "=> ";
    unsigned char sign = (pr_data < 0) ? '-' : '+';
    int percent;

    if (only_val) pr_name = prompt; else pr_name = getmenustr(st_pointer->str_num);
    switch (st_pointer->val_type) {
        case e_empty:
            sprintf(linestr, "%s", pr_name);
            break;
        case e_clatsman:
            // Если указан тип клацман, то печатаем со словом ВКЛ. или Выкл.
            if(pr_data)
                sprintf(linestr, "%sВКЛ.", pr_name);
            else
                sprintf(linestr, "%sВыкл.", pr_name);
            break;
        case e_percent:
            // Если указан тип проценты, то печатаем как проценты
            switch(st_pointer->str_num) {
                case (18): percent = calc_percent(pr_data, prim_par.ADC1_lo, prim_par.ADC1_hi); break;   // Текущее состояние крана (АЦП1)
                case (19): percent = calc_percent(pr_data, PWM_MIN, PWM_MAX); break;   // Заданное состояние крана (PWM1)
                case (20): percent = calc_percent(pr_data, prim_par.ADC2_lo, prim_par.ADC2_hi); break;   // Текущее состояние вентилятора (АЦП2)
                case (21): percent = calc_percent(pr_data, prim_par.PWM2_lo, prim_par.PWM2_hi); break;   // Заданное состояние крана (PWM2)
            };
            //sprintf(linestr, "%s%u.%-01u%%", pr_name, abs(percent/10), abs(percent%10));
            sprintf(linestr, "%s%u%%", pr_name, abs(percent/10));
            break;
        // перенес из boiler-control 15.05.2013
        case e_PWM1: // Если указан тип e_PWM или e_ADC, то печатаем как просто число
        case e_ADC1:
        case e_PWM2:
        case e_ADC2:
            sprintf(linestr, "%s%u", pr_name, pr_data);
            break;
        case e_coef:
            // Если указан тип коэффициент, то печатаем как просто число
            sprintf(linestr, "%s%u.%-01u", pr_name,  abs(pr_data)/10, abs(pr_data%10));
            //sprintf(linestr, "%s%u", pr_name, pr_data);
            break;
        case e_address:
            if (only_val) {
                // printf("[pr_data = %d]", pr_data);
                if (pr_data < 0)
                    sprintf(linestr, "> ...");
                else
                    sprintf(linestr, ">%s", address_to_LCD (mode.ufo[pr_data]));
            } else {
                sprintf(linestr, "%s(%d)", pr_name, mode.new_terms);
            }
            break;
        case e_delete:
            // удаление зарегистрированного термометра
            if (only_val) {
                if (pr_data < 0)
                    sprintf(linestr, "> ...");
                else
                    sprintf(linestr, ">%s", address_to_LCD (ds1820_rom_codes[pr_data]));
            } else {
                // то что рисуется в первой строке
                // printf("[e_delete: pr_data = %02i] ", pr_data);
                sprintf(linestr, "%s", pr_name);
            }
            break;
        case e_scale:
            // Если указан тип шкалы, то печатаем как знаковый байт (-128..127)
            sprintf(linestr, "%s%d", pr_name, pr_data);
            break;
        case e_mode:
            MAIN_MODE = (int)mode.run;
            // Если указан тип режим, то печатаем со словами
            switch (pr_data) {
                case 0: sprintf(linestr, "%sСТОП   ", pr_name); break;  // mo_stop;
                case 1: sprintf(linestr, "%sПУСК   ", pr_name); break;  // mo_action;
                case 2: sprintf(linestr, "%sТО     ", pr_name); break;  // mo_to;
                case 3: sprintf(linestr, "%sПРОГРЕВ", pr_name); break;  // mo_warming_up;
                case 4: sprintf(linestr, "%sОСТАНОВ", pr_name); break;  // mo_warming_down;
                default: break;
            };
            break;
        case e_password:
            // Если указан признак ввода пароля, то просто указываем несколько символов '**'
            if (only_val) sprintf(linestr, "%s%u", pr_name, pr_data);
            else sprintf(linestr, "%s **", pr_name, pr_data);
            break;
        case e_alarm:
            // Если указан признак аварий, то выводим либо слово нет, либо количество аварий и '!'
            if (pr_data) sprintf(linestr, "%s%u!", pr_name, pr_data);
            else sprintf(linestr, "%sНЕТ", pr_name);
            break;
        case e_warning:
            if (pr_data) sprintf(linestr, "%s%u!", pr_name, pr_data);
            else sprintf(linestr, "%s", pr_name);
            break;
        case e_winter:
            // Если указан сезона, то выводим ЗИМА/ЛЕТО
            if (pr_data)
                sprintf(linestr, "%sЗИМА", pr_name);
            else
                sprintf(linestr, "%sЛЕТО", pr_name);
            break;
        case e_time:
            // Если указан тип время, то выбираем из структуры времени и печатаем как чч.мм.сс
            sprintf(linestr, "%s%02u:%02u", pr_name, s_dt.cHH, s_dt.cMM);
            break;
        case e_date:
            // Если указан тип дата, то выбираем из структуры даты и печатаем как дд.мм.гг
            sprintf(linestr, "%s%02u.%02u.%02u", pr_name, s_dt.cdd, s_dt.cmo, s_dt.cyy);
            break;
        case e_dt:
            sprintf(linestr, "%s Вр./Дат.", pr_name);
            break;
        case e_stime:
            // Если указан тип дата, то выбираем из структуры даты и печатаем как дд.мм.гг
            sprintf(linestr, "%s%03u", pr_name, pr_data);
            break;
        case e_temperature:
        case e_room:
        case e_world:
        case e_water:
            // Если указан тип температуры, то печатаем как температуру
            sprintf(linestr, "%s%c%u.%-01uC", pr_name, sign, abs(pr_data)/100, abs(pr_data%100)/10);
            break;
        case e_hour:
        case e_minute:
        case e_day:
        case e_month:
        case e_weekday:
            // Если указан тип [часы, минуты, день, месяц], то печатаем как есть
            sprintf(linestr, "%s%u", pr_name, pr_data);
            break;
        case e_year:
            // Если указан тип год, то печатаем как есть с тысячелетием
            sprintf(linestr, "%s20%u", pr_name, pr_data);
            break;
        default:
            linestr[0] = 0;
    };
    return(linestr);
}
// Функция, инициализирующая menu_level1
void init_curr_menu(struct st_parameter *lmenu, unsigned char lmax) {
    curr_menu.line0 = 0; curr_menu.line1 = 1;

    // curr_menu.menu = &main_menu[0];         // Текущий указатель устанавливаем на главное меню
    curr_menu.menu = lmenu;         // Текущий указатель устанавливаем на главное меню
    // curr_menu.maximum = MENU_LEVEL1;
    curr_menu.maximum = lmax;
    curr_menu.lcd = 0; // curr_menu.value = lmenu->val_data;
}
/* void set_curr_line(unsigned char line) {
//     curr_menu.lcd = line;
//}
// unsigned char get_curr_line(void) {
//    return curr_menu.lcd;
//}
*/
// Функция, модифицирующая структуру содержащую указатели на текущий и следующий элемент. direction - (-1, 0, 1)
void calc_pos(struct st_array_pos *array_pos, byte maxlines, signed char direction) {
    // Увеличиваем или уменьшаем текущий элемент массива строк
    // array_pos->line0 = array_pos->line0 + direction;
    array_pos->line0 += direction;      // выборка элемента структуры по адресу array_pos
    // Начинаем проверки на укладываемость в диапазоне
    if (array_pos->line0 >= maxlines) array_pos->line0 = 0;
    if (array_pos->line0 < 0) { array_pos->line0 = maxlines - 1; array_pos->line1 = 0; return; }
    // Анализируем то, что мы будем возвращать
    if ((array_pos->line0 + 1) == maxlines) array_pos->line1 = 0; else array_pos->line1 = array_pos->line0 + 1;
}
// Функция вывода текущего и следующего пункта меню
void lcd_menu(signed char direction) {
    // printf ("Syncing...\r\n");

    // Если valcoder крутится вправо и курсор был на 1 линии, то ставим ему признак перевода на 2 линию
    if ((curr_menu.lcd == 0) && (direction > 0))    // Анализируем, где находится курсор и куда крутим valcoder
        curr_menu.lcd = 1;
    else
        // Если valcoder крутится влево и курсор был на 2 линии, то ставим ему признак перевода на 1 линию
        if ((curr_menu.lcd == 1) && (direction < 0))
           curr_menu.lcd = 0;
        else
            // Если направление отличается от нуля, то модифицируем текущие пункты меню
            if (direction) calc_pos(&curr_menu, curr_menu.maximum, direction);
    // printf("LCDLine=%u, dir=%u, line0=%u, line1=%u\r\n", curr_menu.lcd, direction, curr_menu.line0, curr_menu.line1);
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 строки 0
    lcd_line_menu(param_str(curr_menu.line0, curr_menu.menu), curr_menu.menu[curr_menu.line0].can_edit);
    lcd_gotoxy(0,1);        // Устанавливаем курсор в позицию 0 строки 1
    lcd_line_menu(param_str(curr_menu.line1, curr_menu.menu), curr_menu.menu[curr_menu.line1].can_edit);
    // Устанавливаем курсор в ту строку, на которую указывает флага curr_menu.lcd
    if (curr_menu.lcd) lcd_gotoxy(LCD_DISP_LENGTH-1,1); else lcd_gotoxy(LCD_DISP_LENGTH-1,0);
    // Даем команду на моргание курсором
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
}
// Функция, инициализирующая режим редактирования
void lcd_init_edit(void) {
    struct st_parameter *lmenu;     // Временная переменная, которая будет ссылаться на выбранный элемент меню
    byte num_line;                  // Временная переменная, содержащая текущий элемент меню
    // Устанавливаем указатель на параметры в зависимости от строки дисплея
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    // Если сейчас нулевой уровень меню и выбранная позиция num_line > 4
    if ((curr_menu.level == 0) && (num_line >= 3)) {
        sync_set_par(SYNC_TO_MENU); // Синхронизируем меню с глобальными структурами
        switch (num_line) {
            case 3:
                    dt_curr_menu = curr_menu;
                    init_curr_menu(&sdt[0], NUM_DT);
                    curr_menu.level = 1; // printf("Вход в меню установки даты и времени\r\n");
                    mode.stop_sync_dt = 1;
                    // sync_set_taym(SYNC_TO_MENU);
                    break;
            case 4: init_curr_menu(&parameters[0], NUM_PARAMETERS);
                    curr_menu.level = 1; // printf("Вход в меню ПАРАМЕТРЫ\r\n");
                    break;
            case 5:
                    if (IS_ALARM) {
                        init_curr_menu(&warnings[0], MAX_WARNINGS);
                        curr_menu.level = 1; // printf("Вход в меню WARNINGS\r\n");
                    }
                    break;
            case 6:
                    if (SETTINGS_OPEN) {
                        init_curr_menu(&settings[0], NUM_SETTINGS);
                        curr_menu.level = 1 ;//printf ("Вход в меню НАСТРОЙКИ\r\n");
                    }
                    break;
            default: ;
        }
        mode.menu = 1;              // Общий режим - меню
        lcd_menu(0);
        return;
    }
    lmenu = &(curr_menu.menu)[num_line];
    if (lmenu->can_edit == 0) {
        lcd_menu(0);
        return;
    } else {
        mode.menu += lmenu->can_edit; // mode.menu = mode.menu + lmenu->can_edit (значение элемента can_edit указателя lmenu на структуру)
    }
    curr_menu.par = lmenu;
    curr_menu.val_data = lmenu->val_data;        // Устанавливаем в текущем меню параметр для редактирования
    lcd_edit(0);
}
// Функция, обрабатывающая выход по Enter
void lcd_save_edit (void) {
    struct st_parameter *lmenu;     // Временная переменная, которая будет ссылаться на выбранный элемент меню
    byte num_line;                  // Временная переменная, содержащая текущий элемент меню
    // Устанавливаем указатель на параметры в зависимости от строки дисплея
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    lmenu = &(curr_menu.menu)[num_line];
    curr_menu.par = lmenu;
    lmenu->val_data = curr_menu.val_data;
    sync_set_par(SYNC_FROM_MENU);
    if (mode.stop_sync_dt) {
        // printf ("Вход в меню main_menu \r\n"); // Синхронизация по выходу
        // init_curr_menu(&main_menu[0], NUM_MENU);
        curr_menu = dt_curr_menu;
        curr_menu.level = 0;
        mode.stop_sync_dt = 0;      // снимаем флаг запрета синхронизации
    }
    // Здесь параметр надо записать в EEPROM !!!!
    if (need_eeprom_write) {
        eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
        // Снимаем флаг необходимости записи в EEPROM
        need_eeprom_write = 0;
    }
    lcd_menu(0);
    return;
}
// Функция, инициализирующая/деинициализирующая режим редактирования (0 - инициализация, 1 - выход по Enter, -1 - выход по Cancel
void lcd_esc_edit(void) {
    struct st_parameter *lmenu;     // Временная переменная, которая будет ссылаться на выбранный элемент меню
    byte num_line;                  // Временная переменная, содержащая текущий элемент меню
    // Устанавливаем указатель на параметры в зависимости от строки дисплея
    if (curr_menu.lcd) num_line = curr_menu.line1; else num_line = curr_menu.line0;
    // Перенесен блок в lcd_init_edit
    lmenu = &(curr_menu.menu)[num_line];
    // Если не нулевой уровень и нажата кнопка Cancel или ev_timer
    // if (mode.print) printf("ESC EDIT: %s = %i-->%i [%u]\r\n", lmenu->val_name, curr_menu.val_data, lmenu->val_data, curr_menu.level);
    if (curr_menu.level || mode.stop_sync_dt) {
        // printf ("Вход в меню main_menu \r\n"); // Синхронизация по выходу
        if (mode.stop_sync_dt)
            curr_menu = dt_curr_menu;
        else
            init_curr_menu(&main_menu[0], NUM_MENU);
        curr_menu.level = 0;
        mode.stop_sync_dt = 0;
        return;
    }
    // Перенесен блок в lcd_init_edit
    curr_menu.par = lmenu;
    curr_menu.val_data = 0;
    lcd_menu(0);
    return;
}
// Функция редактирующая текущий параметр меню. В качестве параметра передается вращение valcoder
void lcd_edit(signed char direction) {
    struct st_parameter *lmenu;
    lmenu = curr_menu.par;
    // printf("EDIT (%d): LCDLine=%u, line0=%u, line1=%u\r\n", direction, curr_menu.lcd, curr_menu.line0, curr_menu.line1);
    // (Де-)Инкрементацию делать надо в зависимости от типа переменной.
    switch (lmenu->val_type) {
        case e_empty:
            // curr_menu.val_data - не меняет свое значение
            break;
        case e_address:
            curr_menu.val_data += (int)direction;
            if (curr_menu.val_data > mode.new_terms - 1) curr_menu.val_data = -1;
            if (curr_menu.val_data < -1) curr_menu.val_data = mode.new_terms - 1;
            break;
        case e_delete:
            if (direction) curr_menu.val_data = ~curr_menu.val_data;
            break;
        case e_clatsman:
        case e_winter:
            // curr_menu.val_data - принимает значение 0 или 1
            if (direction) if(curr_menu.val_data) curr_menu.val_data = 0; else curr_menu.val_data = 1;
            // curr_menu.val_data = ~curr_menu.val_data;
            break;
        case e_mode:
            curr_menu.val_data += (int)direction;
            if (curr_menu.val_data > 2) curr_menu.val_data = 0;
            if (curr_menu.val_data < 0) curr_menu.val_data = 2;
            break;
        case e_temperature:
            // Изменение температуры идет с шагом +/-0,1 градуса
            curr_menu.val_data += (int)direction * 10;
            if (curr_menu.val_data > 10000) curr_menu.val_data = -3000;
            if (curr_menu.val_data < -3000) curr_menu.val_data = 10000;
            break;
        case e_room:
            // Изменение температуры идет с шагом +/-0,1 градуса
            curr_menu.val_data += (int)direction * 10;
            if (curr_menu.val_data > 3000) curr_menu.val_data = 500;
            if (curr_menu.val_data < 500) curr_menu.val_data = 3000;
            break;
        case e_world:
            // Изменение температуры идет с шагом +/-1 градус
            curr_menu.val_data += (int)direction * 100;
            if (curr_menu.val_data > 3000) curr_menu.val_data = -3000;
            if (curr_menu.val_data < -3000) curr_menu.val_data = 3000;
            break;
        case e_water:
            // Изменение температуры идет с шагом +/-1 градус
            curr_menu.val_data += (int)direction * 100;
            if (curr_menu.val_data > 10000) curr_menu.val_data = 500;
            if (curr_menu.val_data < 500) curr_menu.val_data = 10000;
            break;
        case e_percent:
             // Изменение порцентов идет с шагом +/-1 %
            curr_menu.val_data += direction;
            //  curr_menu.val_data += 3*direction;
            //if (curr_menu.val_data > 0xFF) curr_menu.val_data = 0;
            //if (curr_menu.val_data < 0) curr_menu.val_data = 0xFF;
            if (curr_menu.val_data >= PWM_MAX) curr_menu.val_data = PWM_MAX;
            if (curr_menu.val_data <= prim_par.tap_angle) curr_menu.val_data = prim_par.tap_angle;
            break;
        case e_ADC1: // перенес из boiler-control 15.05.2013
            switch_mode_run(mo_setup_input1);
            curr_menu.val_data = ADC_VAR1;
            break;
        case e_ADC2:
            switch_mode_run(mo_setup_input2);
            curr_menu.val_data = ADC_VAR2;
            break;
        case e_PWM1:
            switch_mode_run(mo_setup_output1);
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0 ) curr_menu.val_data = 0;
            if (curr_menu.val_data > 0xFF) curr_menu.val_data = 0xFF;
            break;
        case e_PWM2:
            switch_mode_run(mo_setup_output2);
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0 ) curr_menu.val_data = 0;
            if (curr_menu.val_data > 0xFF) curr_menu.val_data = 0xFF;
            break;
        case e_coef:
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0 ) curr_menu.val_data = 0xFF;
            if (curr_menu.val_data > 0xFF) curr_menu.val_data = 0;
            break;
        case e_scale:
            curr_menu.val_data += direction;
            if (curr_menu.val_data < -128 ) curr_menu.val_data = 127;
            if (curr_menu.val_data > 127) curr_menu.val_data = -128;
            break;
        case e_stime:
            if (curr_menu.val_data < 100) curr_menu.val_data += direction; else
                if (curr_menu.val_data < 1000) curr_menu.val_data += (int)direction*10; else
                    curr_menu.val_data += (int)direction*100;
            if (curr_menu.val_data < 0) curr_menu.val_data = 3600;
            if (curr_menu.val_data > 3600) curr_menu.val_data = 0;
            break;
        case e_minute:          // Время в минутах [00..59]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 59;
            if (curr_menu.val_data > 59) curr_menu.val_data = 0;
            break;
        case e_hour:            // Время в часах [00..23]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 23;
            if (curr_menu.val_data > 23) curr_menu.val_data = 0;
            break;
        case e_day:              // Дни [01..31]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 1) curr_menu.val_data = 31;
            if (curr_menu.val_data > 31) curr_menu.val_data = 1;
            break;
        case e_month:            // Месяцы [01..12]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 1) curr_menu.val_data = 12;
            if (curr_menu.val_data > 12) curr_menu.val_data = 1;
            break;
        case e_year:             // Года [00..99]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 11) curr_menu.val_data = 99;
            if (curr_menu.val_data > 99) curr_menu.val_data = 11;
            break;
        case e_weekday:             // День недели [0..6]
            curr_menu.val_data += direction;
            if (curr_menu.val_data < 0) curr_menu.val_data = 6;
            if (curr_menu.val_data > 6) curr_menu.val_data = 0;
            break;
        case e_warning:
            if (direction) curr_menu.val_data = 0;
            break;
        case e_password:
            curr_menu.val_data += direction;
            SETTINGS_OPEN = (curr_menu.val_data == 20);
            if (curr_menu.val_data < 0) curr_menu.val_data = 99;
            if (curr_menu.val_data > 99) curr_menu.val_data = 0;
            break;
        default:
            curr_menu.val_data += direction;
    }
    // Для системы в целом присваиваем только что измененное значение
    menu_value = curr_menu.val_data;
    // Рисуем две строки.
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 строки 1
    lcd_line_menu(par_str(lmenu, 0, lmenu->val_data), 0);
    lcd_gotoxy(0,1);        // Устанавливаем курсор в позицию 0 строки 1
    lcd_line_menu(par_str(lmenu, 1, curr_menu.val_data), 0);
    lcd_command(LCD_DISP_ON_CURSOR_BLINK);
}
#pragma used+
// Функция печатающая текущий и следующий элемент меню с использованием направления (-1, 0, 1)
void print_curr_menu2(signed char direction) {
    calc_pos(&curr_menu, curr_menu.maximum, direction);
    printf("Выбор меню [%u, %u]", curr_menu.line0, curr_menu.line1);
    printf(" 1) ""%s"",\t2) ""%s""\r\n", main_menu[curr_menu.line0], main_menu[curr_menu.line1]);
}
// Печать всех значений меню параметров в отладочном терминале
void print_curr_menu(void) {
    register byte i;
    // Выведем на отладочную консоль все пункты текущего (!) меню
    printf ("Текущее меню: ");
    for (i=0; i<curr_menu.maximum; i++) printf ("%s\t", param_str(i, curr_menu.menu));
    printf ("\r\n");
}
#pragma used-
// Функция, возвращающая указатель на строку меню по порядковому номеру
char *getmenustr(unsigned char menu_num_pp) {
    static lcd_str menustr;
    if (menu_num_pp) {
        if (menu_num_pp < WARNING_POS)
            strcpyf (menustr, all_menu_str[menu_num_pp-1]);
        else
            strcpy (menustr, get_warning_str(menu_num_pp - WARNING_POS));
        return menustr;
    } else
        return NULL;
}
// Функция преобразования абсолютного значения относительно границ в %
// Возможны ошибки если lo > ( ? >=) hi
int calc_percent(unsigned char x, unsigned char lo, unsigned char hi) {
    long result;
    if (x <= lo )
        result = 0;
    else {
        if (x >= hi ) result = 1000;
            else
            result = (long)(x - lo)* 1000 / (hi - lo);
    }

    return ((int) result);
}
// Функция определяющее поведение меню в случае срабатывания таймера инактивности
void menu_timer_break(void) {
    // Если мы находимся в меню, а не в галвном экране - то заново запускаем таймер неактивности
    if (mode.menu) menu_timer_inactive = TIMER_INACTIVE; // TIMER_INACTIVE;
    switch (mode.menu) {
        case 0: lcd_primary_screen(); break;
        case 1: --mode.menu; lcd_esc_edit(); break;
        case 2: --mode.menu; lcd_esc_edit(); break;
        case 3: mode.menu = 1; break;
    };
    lcd_clrscr();
}