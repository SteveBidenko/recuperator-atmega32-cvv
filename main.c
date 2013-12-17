#include <mega32.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <string.h>
#include <delay.h>
#include "lcd_4bit.h"
#include "menu.h"
#include "robowater.h"
// DS1820 функции температурного преобразования (Биденко+Панарин)
#include "spd1820.h"
#include "valcoder.h"
#include "at2404.h"
#include "alarm.h"
#include "signals.h"
#include "sensors.h"
#include "dayofweek.h"
#include "regular.h"
#include "season.h"
#include "fan.h"
// Локальные макроподстановки
#define MAJOR_VERSION 5
#define MINOR_VERSION 16
// #define NODEBUG
// enum
// Определение главных структур
// Описание глобальны переменных
// int tw_prs;             // Заданная температура воды
// TW_in_Min -Температура Воды на входе( Подаче ) мин. ?
// TW_out_Min -Температура Воды на выходе ( Обратка ) мин.
// TW_out_Stop -Температура Воды на входе для поддержания в режиме стоп.
// TA_in_Min -Температура Воздуха на входе мин -30 С.
// TA_out_Min -Температура Воздуха на выходе мин +15 С.
// TA_out_prs -Температура Воздуха на выходе установленная +20 С.(Заданная)
unsigned char key_treated[7] = {0,0,0,0,0,0,0} ;
struct st_mode mode = {0, mo_stop, 0, 0, 0, 0, 1, 0.0,0,0,0};  // Текущий режим работы
struct st_datetime s_dt;
// Начальные установки структуры основных переменных
struct st_eeprom_par prim_par = {
    48, 205, 0x7F, 0x00,                // П1=50,П2=50,П3=48
    100, 100,                           // int dt_winter, dt_summer;
    (int)10, 0, 0,                      // int Ku, Ki, Kd;
    (int)180, (int)100,
    180,                                // byte T_start
    (int)1500, (int)5000,
    (int)-2000, (int)1000, (int)2200,
    48, 250, 48, 250, 48, 250, 48, 250,  // Установка границ вольтажа входов и выходов
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    1,  // Зима
    0,  // Позиция текущего alarm в EEPROM
    0,   // Сколько всего зарегистрировано в EEPROM
    {2,9,0,0},  //struct st_TO TO; weekday = 2, hour 9 , minute = 0, status = 0;
    MAX_DS1820, // Сколько должно быть термометров в системе
    {
    /*
     {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE}, // 28	4D	52	7E 	3	0	0	6E	1	FF	FE воздух помещение
     {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD}, // 28	D3	8	7E	3	0	0	E4	1	FF	FD   воздух улица
     {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC}, // 28	8C	38	7E	3	0	0	6A	1	FF	FC   вода подача
     {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB} // 28	91	9	7E	3	0	0	32	1	FF	FB   вода обратка
     // Ньютона металоизделия П1
     {0x28,0x29,0x32,0x7e,0x03,0x00,0x00,0x60,0x01}, // 28	29	32	7E 	3	0	0	60	1	FF	FE
     {0x28,0x13,0x3b,0x7e,0x03,0x00,0x00,0xb1,0x01}, // 28	13	3B	7E	3	0	0	B1	1	FF	FD
     {0x28,0x1e,0x2f,0x7e,0x03,0x00,0x00,0x98,0x01}, // 28	1E	2F	7E	3	0	0	98	1	FF	FC
     {0x28,0x39,0x46,0x7e,0x03,0x00,0x00,0x49,0x01} // 28	39	46	7E	3	0	0	49	1	FF	FB
     // Ньютона металоизделия П2
     {0x28,0x95,0x0f,0x7e,0x03,0x00,0x00,0x72,0x01}, // 28	95	F	7E 	3	0	0	72	1	FF	FE
     {0x28,0xc1,0x1a,0x7e,0x03,0x00,0x00,0x2e,0x01}, // 28	C1	1A	7E	3	0	0	2E	1	FF	FD
     {0x28,0x1a,0x17,0x7e,0x03,0x00,0x00,0xfe,0x01}, // 28	1A	17	7E	3	0	0	FE	1	FF	FC
     {0x28,0xe8,0x2a,0x7e,0x03,0x00,0x00,0x8a,0x01} // 28	E8	2A	7E	3	0	0	8A	1	FF	FB
     // Ньютона металоизделия П3
     {0x28,0x50,0x32,0x7e,0x03,0x00,0x00,0x6e,0x01}, // 28	50	32	7E 	3	0	0	6E	1	FF	FE
     {0x28,0x3e,0x1c,0x7e,0x03,0x00,0x00,0xe4,0x01}, // 28	3E	1C	7E	3	0	0	E4	1	FF	FD
     {0x28,0xb7,0x0d,0x7e,0x03,0x00,0x00,0x29,0x01}, // 28	B7	D	7E	3	0	0	29	1	1	FF	FC
     {0x28,0x2c,0x0c,0x7e,0x03,0x00,0x00,0xad,0x01} // 28	2C	C	7E	3	0	0	AD	1	FF	FB
     // Ньютона металоизделия П4
     {0x28,0x4d,0x52,0x7e,0x03,0x00,0x00,0x6e,0x01}, // 28	4D	52	7E 	3	0	0	6E	1	FF	FE
     {0x28,0xd3,0x08,0x7e,0x03,0x00,0x00,0xe4,0x01}, // 28	D3	8	7E	3	0	0	E4	1	FF	FD
     {0x28,0x8c,0x38,0x7e,0x03,0x00,0x00,0x6a,0x01}, // 28	8C	38	7E	3	0	0	6A	1	FF	FC
     {0x28,0x91,0x09,0x7e,0x03,0x00,0x00,0x32,0x01}} // 28	91	9	7E	3	0	0	32	1	FF	FB
     // 50 ЛетВЛКСМ бильярдная.
     {0x28,0x1b,0xc4,0x24,0x00,0x00,0x00,0xfe,0x01}, // 28	1b	c4	24	0	0	0	fe	1	FF	FE
     {0x28,0xbb,0x0a,0x95,0x03,0x00,0x00,0xd8,0x01}, // 28	bb	a	95	3	0	0	d8	1	FF	FD
     {0x22,0x8e,0xe1,0x03,0x00,0x00,0x00,0xf3,0x01}, // 22	8e	e1	3	0	0	0	f3	1	FF	FC
     {0x22,0x2b,0x08,0x04,0x00,0x00,0x00,0xeb,0x01} // 22	2b	8	4	0	0	0	eb	1	FF	FB
     // Микрофарм П2.
     {0x28,0xf8,0x28,0x94,0x03,0x00,0x00,0x00,0x01}, // 28  F8  28  94  3   0   0   0   1	FF	FE   Помещение
     {0x28,0x7b,0x3b,0x94,0x03,0x00,0x00,0x81,0x01}, // 28  7B  3B  94  3   0   0   81  1	FF	FD   Улица
     {0x28,0x54,0x26,0x94,0x03,0x00,0x00,0x83,0x01}, // 28  54  26  94  3   0   0   83  1	FF	FC   Вода ВХОД
     {0x28,0xe7,0x3f,0x94,0x03,0x00,0x00,0x52,0x01}  // 28  E7  3F  94  3   0   0   52  1	FF	FB   Вода выход
     // Микрофарм П1.
     {0x28,0xba,0x56,0x7e,0x03,0x00,0x00,0x86,0x01}, // 28  BA  56  7E  3   0   0   86  1	FF	FE   Помещение
     {0x28,0xea,0x20,0x95,0x03,0x00,0x00,0x84,0x01}, // 28  EA  20  95  3   0   0   84  1	FF	FD   Улица
     {0x28,0x1e,0x34,0x7e,0x03,0x00,0x00,0x94,0x01}, // 28  1E  34  7E  3   0   0   94  1	FF	FC   Вода ВХОД
     {0x28,0x1e,0x2b,0x7e,0x03,0x00,0x00,0x87,0x01}  // 28  1E  2B  7E  3   0   0   87  1	FF	FB   Вода выход
     // Зеньков П1.
     {0x28,0x61,0x23,0x7e,0x03,0x00,0x00,0x05,0x01}, // 28  61  23  7E  3   0   0   5   1	FF	FE   Помещение
     {0x28,0x56,0x21,0x7e,0x03,0x00,0x00,0xee,0x01}, // 28  56  21  7E  3   0   0   EE  1	FF	FD   Улица
     {0x28,0x13,0x57,0x7e,0x03,0x00,0x00,0x81,0x01}, // 28  13  57  7E  3   0   0   81  1	FF	FC   Вода ВХОД
     {0x28,0xc6,0x1e,0x7e,0x03,0x00,0x00,0xb4,0x01}  // 28  C6  1E  7E  3   0   0   B4 1	FF	FB   Вода выход
     // Зеньков П2.
     {0x28,0xb8,0x24,0x7e,0x03,0x00,0x00,0x06,0x01}, // 28  B8  24  7E  3   0   0   6   1	FF	FE   Помещение
     {0x28,0x2c,0x1d,0x7e,0x03,0x00,0x00,0x1c,0x01}, // 28  2C  1D  7E  3   0   0   1C  1	FF	FD   Улица
     {0x28,0x6f,0x42,0x7e,0x03,0x00,0x00,0xca,0x01}, // 28  6F  42  7E  3   0   0   CA  1	FF	FC   Вода ВХОД
     {0x28,0x4f,0x36,0x7e,0x03,0x00,0x00,0x0e,0x01}  // 28  4F  36  7E  3   0   0   E   1	FF	FB   Вода выход
     // Зеньков П3.
     {0x28,0xc2,0x14,0x7e,0x03,0x00,0x00,0xd5,0x01}, // 28  C2  14  7E  3   0   0   D5  1	FF	FE   Помещение
     {0x28,0xd6,0x3a,0x7e,0x03,0x00,0x00,0x08,0x01}, // 28  D6  3A  7E  3   0   0   8   1	FF	FD   Улица
     {0x28,0x4f,0x4c,0x7e,0x03,0x00,0x00,0xde,0x01}, // 28  4F  4C  7E  3   0   0   DE  1	FF	FC   Вода ВХОД
     {0x28,0x0a,0x3e,0x7e,0x03,0x00,0x00,0xae,0x01}  // 28  A   3E  7E  3   0   0   AE  1   FF	FB   Вода выход
     // Лабораторный вариант 1
     {0x28,0x6D,0x3F,0x94,0x03,0x00,0x00,0x77,0x01}, // 28  6D  3F  94  03  00  00  77  1	FF	FE   Помещение
     {0x28,0x9D,0x41,0x94,0x03,0x00,0x00,0xCA,0x01}, // 28  9D  41  94  03  00  00  CA  1	FF	FD   Улица
     {0x28,0xF7,0xFF,0x93,0x03,0x00,0x00,0xAD,0x01}, // 28  F7  FF  93  03  00  00  AD  1	FF	FC   Вода ВХОД
     {0x28,0xA9,0x29,0x94,0x03,0x00,0x00,0xD4,0x01}  // 28  A9  29  94  03  00  00  D4  1   FF	FB   Вода выход

     // Лабораторный вариант 2
     {0x28,0xc2,0x14,0x7e,0x03,0x00,0x00,0xd5,0x01}, // 28  C2  14  7E  3   0   0   D5  1	FF	FE   Помещение
     {0x28,0xd6,0x3a,0x7e,0x03,0x00,0x00,0x08,0x01}, // 28  D6  3A  7E  3   0   0   8   1	FF	FD   Улица
     {0x28,0x4f,0x4c,0x7e,0x03,0x00,0x00,0xde,0x01}, // 28  4F  4C  7E  3   0   0   DE  1	FF	FC   Вода ВХОД
     {0x28,0x0a,0x3e,0x7e,0x03,0x00,0x00,0xae,0x01}  // 28  A   3E  7E  3   0   0   AE  1 FF	FB   Вода выход
     //Краснооктябрьская 90 ПВ1
     {0x28,0x6a,0x22,0x94,0x03,0x00,0x00,0x62,0x01}, // 28  6A  22  94  03  00  00  62  1	FF	FE   Помещение
     {0x28,0x93,0x10,0x94,0x03,0x00,0x00,0x81,0x01}, // 28  93  10  94  03  00  00  81  1	FF	FD   Улица
     {0x28,0x03,0x42,0x94,0x03,0x00,0x00,0x26,0x01}, // 28  03  42  94  03  00  00  26  1	FF	FC   Вода ВХОД
     {0x28,0xe4,0x09,0x94,0x03,0x00,0x00,0x13,0x01}  // 28  E4  09  94  03  00  00  13  1 FF	FB   Вода выход  
     */
     // Донецк салон PORSHE
     {0x28,0x00,0x12,0x94,0x03,0x00,0x00,0xea,0x01}, // 28  00  12  94  03  00  00  EA  1	FF	FE   Помещение
     {0x28,0xab,0x42,0x94,0x03,0x00,0x00,0xdb,0x01}, // 28  AB  42  94  03  00  00  DB  1	FF	FD   Улица
     {0x28,0xbd,0xf6,0x93,0x03,0x00,0x00,0xe4,0x01}, // 28  BD  F6  93  03  00  00  E4  1	FF	FC   Вода ВХОД
     {0x28,0x02,0xf5,0x93,0x03,0x00,0x00,0x89,0x01}  // 28  02  F5  93  03  00  00  89  1   FF	FB   Вода выход

    }
};
unsigned char size_prim_par;        // Почти константа. Нужна для функций записи/чтения из/в EEPROM

int time_cooling = 0;
enum en_event event;                          // Текущее событие в системе
#define HELP_LINES 14
typedef char help_str[70];
flash help_str all_help_str[HELP_LINES] = {
"z - общее состояние системы",                                          // [0]
"s - переключение звука",                                               // [1]
"w - печать номера текущего события",                                   // [2]
"x - переключение печати",                                              // [3]
"v - печать адресов термометров (ds1820_rom_codes)",                    // [4]
"b - печать адресов не определенных термометров (ufo)",                 // [5]
"e - печать журнала последних аварий",                                  // [6]
"d - удаление тревог",                                                  // [7]
"\\ - ручной поиск новых термометров",                                  // [8]
"с - печать структуры prim_par",                                        // [9]
"m - печать текущих параметров системы",                                // [10]
"n - печать состояния системы",                                         // [11]
"0..3 - поиск адреса термометра из prim_par.addr в ds1820_rom_codes",   // [12]
"9 - вывод всей структуры сигналов",                                    // [13]
};
// Описание функций
void print_help();
void printallterms(unsigned char);
void printnewterms(void);
void print_prim_par(unsigned char *, unsigned char);
// Функции основного цикла
void check_serial(void);
void check_peripheral(void);
void event_processing(void);
void mode_processing(void);
// void delay_run(void);
void start_screen(unsigned char);
void full_reset(void);          // функция, востанавливающая все (!) к заводским настройкам
void factory_reset(void);       // Функция, востанавливающая установки по умолчанию не трогая информацию о термометрах
void factory_term_reset(void);  // Функция, восстанавливающая настройки из EEPROM и записывающая адреса заводских термометров в EEPROM
// Основная программа
void main(void) {
    // register byte i;
    init();                  // Инициализация всей периферии
    #asm("sei")             // Global enable interrupts
    init_keys();
    // Сохраняем в EEPROM структуру prim_par
    size_prim_par = sizeof(prim_par);
    // СБРОС параметров в EEPROM к заводским установкам, если одновременно нажаты клавиши CANCEL + ENTER
    if (!KEY_CANCEL && !KEY_ENTER) {
        full_reset();
    } else {
        if (!KEY_CANCEL) factory_reset();
        if (!KEY_ENTER) factory_term_reset();
    }
    // delay_run(); временно отключили
    // Восстанавливаем из EEPROM структуру prim_par со всеми термометрами
    eeprom_read_struct ((char *)&prim_par, size_prim_par);
    // Инициализируем все термометры и запускаем на измерение
    read_all_terms(INIT_MODE);
    start_screen(0);
    // print_all_menu();       // Выведем на отладочную консоль все пункты меню
    sync_set_par(SYNC_TO_MENU); // Синхронизируем меню с глобальными структурами
    update_warning_menu();        // Обновляем меню warnings
    //printf("После чтение из EEPROM значение Tw2=%u\r\n", prim_par.Tw2);
    // printf("Было %u, стало после чтение из EEPROM size=%u\r\n", sizeof(prim_par), size_prim_par);
    #ifndef NODEBUG
    // Если есть термометры, то выводим их значение
    printallterms(0);
    #endif
    lcd_primary_screen();       // выводим стартовую картинку на экранчик
    if (prim_par.season) signal_white(ON); else signal_white(OFF);
    FAN_SPEED = check_fan_range(prim_par.fan_speed);
    link_terms();
    // Вычисляем значение крана по наружной температуре
    tap_angle_check_range(INIT_MODE);
    while(1) {
        // ВНИМАНИЕ! НИЖЕ ДО ОКОНЧАНИЯ ЦИКЛА WHILE КОД НЕ ДОБАВЛЯТЬ!!!
        check_serial();
        poll_keys();
        /* if (event == ev_none) */ check_peripheral();
        /* if (0 == 1) */ event_processing();
        mode_processing();
    }; // while (1)

} // void main(void)
// Функция опроса перефирии и генерации событий по результатом опроса, а также по результатам МАТАнализа
void check_peripheral(void) {
    // В этой функции запрещено (!) устанавливать режимы
    // Обрабатываем поворот valcoder'а
    if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {        // Если сработал valcoder
        if (valcoder < 0)
            event = ev_left;
        else
            event = ev_right;
    }
    // Если поворота не было, опрашиваем другие порты
    if (CHECK_EVENT && (!KEY_ENTER)) event = ev_enter;
    if (CHECK_EVENT && (!KEY_CANCEL)) event = ev_cancel;
    // Обслуживаем кнопку стоп
    //if (CHECK_EVENT && (!timer_stop) && (!KEY_STOP)) event = ev_stop;
    if (CHECK_EVENT &&  (!KEY_STOP)) event = ev_stop;
    // Обслуживаем кнопку старт, Проверяя при этом наличие каких-либо аварий
    if (CHECK_EVENT && (IS_ALARM == 0) && (!KEY_START)) event = ev_start;
    // проверяeм термометры, наличие warnings и alarms
    regular_check_alarm_and_warnings();
    // Проверяем, что из меню была задана команда ПУСК, СТОП или ТО
    if (CHECK_EVENT && (mode.initrun)) {
        mode.initrun -= INITMODE;
        switch (mode.initrun) {
            case mo_stop:
                event = (mode.run == mo_to) ? ev_end_to : ev_stop;
                break;
            case mo_action:
                if ((IS_ALARM == 0) && mode.run == mo_stop) {
                    event = ev_start;
                }
                break;
            case mo_to:
                if ((IS_ALARM == 0) && mode.run == mo_stop) {
                    event = ev_begin_to;
                }
                break;
            default:
        }
        mode.initrun = 0;
    }
    #ifndef NODEBUG
    // if (event) printf ("Сгенерировано новое событие: %u\r\n", (unsigned char)event);
    //if (event) printf ("-");
    #endif
}
// Обработка событий в системе
void event_processing(void) {
    // Только здесь можно менять режимы. Но здесь нельзя опрашивать периферию, делать матанализ
    #ifndef NODEBUG
    // if (event != ev_none)
        // printf ("%02u:%02u Текущее событие = %u\r\n", s_dt.cMM, s_dt.cSS, (unsigned char)event);
    #endif
    // Также здесь выполняются инициализационные действия для процессов: Вкл./выкл индикатор, запустить бибикалку, нарисовать строку меню и т.п.
    switch (event) {
        case ev_secunda:                // Обрабатываем ежесекундное событие.
            regular_inspection();       // Реализовано отдельной функцией в main.c
            break;
        case ev_left:                   // printf ("Обрабатываем прокрутку valcoder влево\r\n");
        case ev_right:                  // printf ("Обрабатываем прокрутку valcoder вправо\r\n");
            // Запускаем таймер инактивности
            menu_timer_inactive = TIMER_INACTIVE;
            valcoder = VALCODER_NO_ROTATE;
            signal_buz(OFF);
            // printf ("Обрабатываем прокрутку valcoder (%d), в режиме %d - ", event-2, mode.menu);
            switch (mode.menu) {
                case 0:
                    sync_set_par(SYNC_TO_MENU);
                    lcd_menu(mode.menu++); break;   // Выводим меню без изменения позиции printf ("entering...");
                case 1: lcd_menu(event-2); break;       // printf ("navigating...");
                case 2: lcd_edit(event-2); break;       // printf ("editing...");
                default: ;                              // printf ("defaulting...");
            }
            // printf ("\r\n");
            event = ev_none;            // Очищаем событие
            break;
        case ev_enter:                  // Если нажат Enter
            // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW; if (mode.menu <= 2)
            key_treated[2]=1;
            event = ev_none;            // Очищаем событие
            signal_buz(SHORT);
            menu_timer_inactive = TIMER_INACTIVE;     //Запускаем таймер инактивности
            switch (mode.menu) {
                // lcd_primary_screen();
                // Обрабатываем нажатие enter c учетом того, что значение mode.menu еще старое
                case 0:
                    sync_set_par(SYNC_TO_MENU);
                    lcd_menu(mode.menu++); break;           // Если находились в главном экране, запускаем прорисовку меню
                case 1: lcd_init_edit(); break;             // Если находились в меню, то анализируем
                case 2: lcd_save_edit(); mode.menu = 1; break;
            };
            break;
        case ev_cancel:
            menu_timer_break();
            signal_buz(SHORT);
            key_treated[3]=1;
            event = ev_none;            // Очищаем событие
            break;
        case ev_start:      // пуск установки
            key_treated[1]=1;
            printf ("Нажата кнопка ПУСК. ");
            switch (mode.run) {
                case mo_stop: // Если сейчас режим STOP
                    if (prim_par.season) {
                        signal_white(ON);
                        mode.pomp = 1;
                        mode.run = mo_warming_up;     // Устанавливаем режим Прогрев
                        timer_start = prim_par.T_start; // Запускаем таймер STRT
                        TAP_ANGLE = PWM_MAX;
                        printf("Включен режим Прогрев. Время прогрева = %d\r\n", timer_start);
                        signal_green(SHORT);
                    } else {
                        signal_white(OFF);
                        mode.pomp = 0;
                        mode.run = mo_action;
                        mode.fan = 1;
                        signal_green(ON);
                        printf("Включен режим Пуск\r\n");
                    };
                    FAN_SPEED = check_fan_range(prim_par.fan_speed);
                    //count_fan = 0;
                    time_integration = 0;
                    //time_cooling = TIME_COOLING_MAX;
                    time_cooling = prim_par.T_z;
                    signal_buz(LONG);
                    break; // mo_stop
                case mo_warming_up:
                    signal_buz(SHORT);
                    if (mode.print) printf("Режим Прогрев. Время прогрева = %d\r\n", timer_start);
                    break; // mo_warming_up
                case mo_action: signal_buz(SHORT);
                    if (mode.print) printf("Режим Пуск.\r\n");
                    break; // mo_action
                default: break;
            };
            event = ev_none;
            break;
        case ev_stop:
            key_treated[0]=1;
            printf ("Нажата кнопка СТОП. ");
            switch (mode.run) {
                case mo_stop: signal_buz(SHORT); time_integration = 0;
                    if (mode.print) printf(" Режим СТОП.\r\n");
                    break; // mo_stop
                case mo_warming_up: 
                    mode.run = mo_warming_down;
                    timer_start = 0;
                    signal_green(SHORT);  signal_buz(LONG);
                    if (mode.print) printf ("Включен режим Остановки\r\n");
                    timer_stop = TIME_STOP;    // Запускаем таймер STOP
                    time_integration = 0;
                    break; //mode.run=1;
                case mo_warming_down:
                    signal_buz(SHORT);
                    time_integration = 0;
                    if (mode.print) printf("Режим Остановки.\r\n");
                    break; // mo_warming_down
                case mo_action:
                    mode.run = mo_warming_down;
                    signal_green(LONG); signal_buz(LONG);
                    if (mode.print) printf ("Включен режим Остановки\r\n");
                    time_cooling = 0;
                    timer_stop = TIME_STOP;    // Запускаем таймер STOP
                    time_integration = 0;
                    break; // mo_action
                case mo_to:
                    printf ("Принудительное окончание ТО\n");
                    timer_start = 0;
                    time_integration = 0;
                    prim_par.TO.status = 0;
                    timer_stop_to = 0;
                    timer_start_to = 0;
                    break; // mo_to
                default: break;
            }
            mode.fan = 0;  // Выключение Вентилятора
            event = ev_none;
            break;
        case ev_begin_to:
            if (mode.run == mo_stop) {
                mode.run = mo_to;
                timer_start_to = T_TO; // Запускаем таймер STRT
                timer_stop_to = 0;
                TAP_ANGLE = PWM_MAX;
                printf("ТО КРАНА. Открывание LIMIT = %d, Время на открывание = %d\n", TAP_ANGLE, timer_start_to);
            }
            event = ev_none;
            break;
        case ev_to_nf:
            alarm_reg(0, 1, get_warning_str(6), 6);
            printf("Невозможно провести ТО КРАНА." );
        case ev_end_to:
            timer_start_to = 0;
            timer_stop_to = 0;
            switch_mode_run(mo_stop);
            event = ev_none;
            prim_par.TO.status = 0;
            printf("Конец ТО КРАНА." );
            break;
        case ev_alarm1:   // Пожар, перегрев вентилятора, авария частотника
            signal_red(ON); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1; // Проверить
            TAP_ANGLE = PWM_MAX;
            alarm_reg(0, 1, get_warning_str(0), 0);
            printf ("Авария: %s\r\n", get_warning_str(0));
            event = ev_none;
            break;
        case ev_alarm2:   // Угроза замораживания от внешнего датчика
            signal_red(ON); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            alarm_reg(0, 1, get_warning_str(1), 1);
            printf ("АВАРИЯ: %s\r\n", get_warning_str(1));
            event = ev_none;
            break;
        case ev_freezing1:  // Температура на улице ниже критической UL_T (Если не повторяется в течении часа восстановление)
            alarm_reg(0, 1, get_warning_str(2), 2);
            signal_red(ON);
            signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(2));
            event = ev_none;
            break;
        case ev_freezing2: // Температура в помещения ниже критической POM_T
            alarm_reg(0, 1, get_warning_str(3), 3);
            signal_red(ON); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(3));
            event = ev_none;
            break;
        case ev_freezing3: // Температура воды обратки ниже критической POM_T
            alarm_reg(0, 1, get_warning_str(5), 5);
            signal_red(ON); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(5));
            event = ev_none;
            break;
        case ev_term1_nf:   // Термометр В1 помещение
            alarm_reg(MAX_OFFLINES, termometers[0].err, get_warning_str(7), 7);
            signal_red(ON); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(7));
            event = ev_none;
            break;
        case ev_term2_nf:  // Термометр В2 Улица
            alarm_reg(MAX_OFFLINES, termometers[1].err, get_warning_str(8), 8);
            signal_red(LONG); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(8));
            event = ev_none;
            break;
        case ev_term3_nf:  // Термометр В3 Подача
            // Нет его, ну и хрен с ним
            alarm_reg(MAX_OFFLINES, termometers[2].err, get_warning_str(9), 9);
            printf ("ПРЕДУПРЕЖДЕНИЕ: %s\r\n", get_warning_str(9));
            event = ev_none;
            break;
        case ev_term4_nf:  // Термометр В4 Обратка
            alarm_reg(MAX_OFFLINES, termometers[3].err, get_warning_str(10), 10);
            signal_red(LONG); signal_buz(MEANDR);
            switch_mode_run(mo_stop);
            if (prim_par.season) mode.pomp = 1;
            TAP_ANGLE = PWM_MAX;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(10));
            event = ev_none;
            break;
        case ev_filter:  // Загрязнение фильтра.
            alarm_reg(0, 1, get_warning_str(11), 11);
            event = ev_none;
            printf ("АВАРИЯ: %s\r\n", get_warning_str(11));
            signal_red(SHORT);
            break;
        default:
            break;
    };
}
// Обработка режима работы системы
void mode_processing(void) {
    // В ЭТОЙ ФУНКЦИИ ЗАПРЕЩЕНО ИЗМЕНЕНИЕ РЕЖИМОВ И ГЕНЕРАЦИЯ СОБЫТИЙ. ТОЛЬКО ПОДДЕРЖКА!
    SET_T = prim_par.TA_out_prs;
    // Здесь выставляются значения на лапах контроллера
    switch (mode.run) {
        case mo_stop:
        case mo_warming_up:
        case mo_warming_down:
        case mo_action:
           OCR0 = (unsigned char)TAP_ANGLE;
           OCR2 = (unsigned char)FAN_SPEED;// Задействованно для включения охладителя
           FAN = FAN_MENU = mode.fan;
           POMP = POMP_MENU = mode.pomp;
           // COOLING1 = mode.cooling1;
           // COOLING2 = mode.cooling2;
           break;
        case mo_to:
            if ((timer_stop_to == 0) && (ADC_VAR1 >= (prim_par.ADC1_hi - 10))) {
               TAP_ANGLE = PWM_MIN;
               timer_start_to = 0;
               timer_stop_to = T_TO;
               printf("ТО КРАНА. Закрывание LIMIT = %d, Время на закрывание = %d\n", TAP_ANGLE, timer_stop_to);
            }
            OCR0 = (unsigned char)TAP_ANGLE;
            break;
        case mo_setup_input1:
            break;
        case mo_setup_input2:
            break;
        case mo_setup_output1:
            OCR0 = (unsigned char)menu_value;
            break;
        case mo_setup_output2:
            OCR2 = (unsigned char)menu_value;
            break;
        default:

    }
}
/*         */
void switch_mode_run(enum en_mode newstatus) {
    if ((unsigned char)mode.run <= 4) mode.lastrun = mode.run;
    mode.run = newstatus;
    switch (mode.run) {
        case mo_stop:
            signal_green(OFF);
            mode.fan = 0;           // Выключение Вентилятора
        case mo_action:
            signal_green(ON);
            break;
        default: break;
    } 
} 
// Печать всех термометров. mode - режим печати адресов термометров. Если не 0 - печатать адреса.
void printallterms(unsigned char print_addr) {
    int term;
    register byte i;
    unsigned char resolution;
    signed char th, tl;
    unsigned char *spd, *address_line;

    if(!ds1820_devices) return;			// если термометры не обнаружены - просто выходим из функции
    spd = ds1820_show_spd();
    if (print_addr) printf("Адреса термометров(ds1820_rom_codes): ");
    for(i=0; i<ds1820_devices; i++) {
        ds1820_get_resolution(&ds1820_rom_codes[i][0], &resolution);
    	term = ds1820_temperature(ds1820_rom_codes[i]);
    	// эквивалентно term = ds1820_temperature(&ds1820_rom_codes[i][0]);
        tl = th = 0;
        ds1820_get_alarm(ds1820_rom_codes[i], &tl, &th);
        // эквивалентно ds1820_get_alarm(&ds1820_rom_codes[i][0], &tl, &th);
        if (print_addr) {
            address_line = address_to_LCD (ds1820_rom_codes[i]);
            printf("%s:%d ", address_line, termometers[i].err);
        } else {
            printf(" t%-u = %i(%-i.%-u)C[%02X%02X:%02x] err:%d; ", i+1, term, term/100, abs(term%100), spd[1], spd[0], resolution, termometers[i].err);
        }
    }
    printf("\r\n");
}
// Печать всех новых термометров
void printnewterms(void) {
    if (mode.new_terms) {
        register byte i;
        unsigned char *address_line;

        printf("mode.ufo Неизвестных термометров new_terms = %d: ", mode.new_terms);
        for (i = 0; i < mode.new_terms; i++) {
            address_line = address_to_LCD (mode.ufo[i]);
            printf("[ %s ] ", address_line);
        }
        printf("\r\n");
    } else
        printf("Новых термометров не найдено\r\n");
}
// Функция высвечивающая версию программы на LCD и на терминале
void start_screen(unsigned char init) {
    lcd_clrscr();
    printf("Старт П1. %u.%02u. Найдено %u термометров.\r\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    lcd_command(LCD_DISP_ON);       // Убираем курсор с LCD
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 первой строки
    sprintf(linestr, "MTECH %u.%02u",MAJOR_VERSION, MINOR_VERSION );
    lcd_puts(linestr);
    lcd_gotoxy(0,1);                // Устанавливаем курсор в позицию 0 строки 2
    switch (init) {
        case 1:
            sprintf(linestr, "FULL RESET"); break;
        case 2:
            sprintf(linestr, "FACTORY"); break;
        case 3:
            sprintf(linestr, "ONLY NEW TERMS"); break;
        case 0:
        default:
            sprintf(linestr, "Robowater ..."); break;
    }
    lcd_puts(linestr);
}
// Печать структуры prim_par
void print_prim_par(unsigned char *struct_data, unsigned char size) {
    register unsigned char i;
    unsigned char s_byte;
    printf("Состояние PRIM_PAR [%d bytes]: ", size);
    for (i=0; i<size; i++) {
        s_byte = *struct_data++;
        if (i <= (size - 37))		// 37 - это смещение массива термометров в prim_par от конца
            printf(" %d", s_byte);
        else
            printf("%02X ", s_byte);
        if (i == size - 37) printf("\r\nTERMS: ");
    }
    printf("\r\n");
}
// Переключение звука
void toggle_sound(void) {
    if (mode.sound)
        printf("Звук выключен\r\n");
    else
        printf("Звук включен\r\n");
    mode.sound = !mode.sound;
}
// Старт программы с задержкой
// #define DELAY_TIME 300
#define DELAY_TIME 30
/*
void delay_run(void) {
    byte i =0;
    signal_buz(LONG);
    for (i = 0; i < 7; i++){
        delay_ms(DELAY_TIME);
        signal_white(ON);
        delay_ms(DELAY_TIME);
        signal_white(OFF);
        signal_red(ON);
        delay_ms(DELAY_TIME);
        signal_red(OFF);
        signal_green(ON);
        delay_ms(DELAY_TIME);
        signal_green(OFF);
    };

}
*/
// Переключение печати
void toggle_print(void) {
    mode.print = !mode.print;
    if (mode.print)
        printf("Печать включена\r\n");
        //printf("Печать выключена\r\n Время, Температура заданная, Температура измеренная, Delta, Угол крана расчетный, Угол кранаизмеренный, Угол ограничения, result, pTerm, iTerm, dTerm, Обороты расчетные, Обороты измеренныее\r\n");
    else
        //printf("Печать выключена\r\n Время, Температура заданная, Температура измеренная, Delta, Угол крана расчетный, Угол кранаизмеренный, Угол ограничения, result, pTerm, iTerm, dTerm, Обороты расчетные, Обороты измеренныее\r\n");
        printf("Печать выключена\r\n");
}
// Обработка событий от серийного порта
void check_serial(void) {
    byte inbyte;    // Описание локальных переменных
    register byte i;
    // Обрабатываем последовательный порт
    if (UCSRA & RX_COMPLETE) { // Пришло ли что-нибудь
        inbyte = UDR;
        switch (inbyte) {
            case 0x68:  /* 'h' */
                print_help(); break;
            case 0x7A:  /* 'z' */
                printf("Время: %02u:%02u:%02u, дата:%02u.%02u.%02u, найдено %u термометров\r\n",
                        s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.cyy, ds1820_devices);
                printallterms(0);
                break;
            case 0x73:  /* 's' */     // переключение звука
                toggle_sound(); break;
            case 0x77:  /* 'w' */     // символ
                printf ("Cобытие = %u\r\n", (unsigned char)event);
                break;
            case 0x78:  // символ 'x'
                toggle_print(); break;
            case 0x76: // символ 'v'
                printallterms(1); break;
            case 0x62: // символ 'b'
                printnewterms(); break;
            case 0x65:  // символ 'e'
                alarm_all_print(); break;
            case 0x64:  // символ 'd'
                printf ("Проверяем список тревог.\r\n");
                for (i=0; i < MAX_WARNINGS; i++) {
                    if (!(alarm_unreg(i))) {
                       printf (" Удалили активную тревогу: %s\r\n", get_warning_str(i));
                    }
                }
                break;
            case 0x5c:  // символ '\'
                printf ("Инициализируем все термометры!\r\n");
                read_all_terms(INIT_MODE);
                break;
            case 0x63:                // символ 'c'
                print_prim_par((unsigned char *)&prim_par, sizeof(prim_par));
                break;
            case 0x6D:                // символ 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\r\n");
                break;
            case 0x6E:                // символ 'n'
                for(i=0; i<NUM_MENU; i++) printf("%s\t", param_str(i, main_menu));
                printf("\r\n");
                break;
            case 0x30:              // символ '0'
                i = ds1820_is_exist (prim_par.addr[0], ds1820_rom_codes[0]);
                printf ("%s термометр найден в позиции %u\r\n", address_to_LCD(prim_par.addr[0]), i);
                break;
            case 0x31:              // символ '1'
                i = ds1820_is_exist (prim_par.addr[1], ds1820_rom_codes[0]);
                printf ("%s термометр найден в позиции %u\r\n", address_to_LCD(prim_par.addr[1]), i);
                break;
            case 0x32:              // символ '2'
                i = ds1820_is_exist (prim_par.addr[2], ds1820_rom_codes[0]);
                printf ("%s термометр найден в позиции %u\r\n", address_to_LCD(prim_par.addr[2]), i);
                break;
            case 0x33:              // символ '3'
                i = ds1820_is_exist (prim_par.addr[3], ds1820_rom_codes[0]);
                printf ("%s термометр найден в позиции %u\r\n", address_to_LCD(prim_par.addr[3]), i);
                break;
            case 0x39:              // символ '9'
                signal_printallbytes();
                break;
            default:
                printf("Нажат символ 0x%x\r\n", inbyte);
        };
    }
}
// функция, востанавливающая все (!) к заводским настройкам
void full_reset(void) {
    start_screen(1);
    printf("Запись в EEPROM всех (!) заводских установок.\r\n");
    // Сихронизация массива термометров с EEPROM
    eeprom_write_struct ((unsigned char *)&prim_par, size_prim_par);
}
// Функция, востанавливающая установки по умолчанию не трогая информацию о термометрах
void factory_reset(void) {
    unsigned char size_addr = size_prim_par - sizeof(prim_par.addr);

    // При нажатой клавише CANCEL записываем установки по умолчанию не трогая информацию о термометрах
    start_screen(2);
    ds1820_devices = MAX_DS1820;
    printf("Запись в EEPROM установок по умолчанию.\r\n");
    eeprom_write_struct ((unsigned char *)&prim_par, size_addr);
}
// Функция, восстанавливающая натройки из EEPROM и записывающая рассортированые термометры обратно
void factory_term_reset(void) {
    unsigned char size_addr = size_prim_par - sizeof(prim_par.addr);

    start_screen(3);
    printf("Вычитываем из EEPROM настройки без адресов термометра\r\n");
    eeprom_read_struct ((char *)&prim_par, size_addr);
    printf("Запись в EEPROM адресов заводских термометров.\r\n");
    eeprom_write_struct ((unsigned char *)&prim_par, size_prim_par);
}
// Функция печати на экран HELP информации
void print_help() {
    static help_str curr_line;
    register unsigned char i;
    for (i = 0; i < HELP_LINES; i++) {
        strcpyf(curr_line, all_help_str[i]);
        printf("%s\r\n", curr_line);
    }
}