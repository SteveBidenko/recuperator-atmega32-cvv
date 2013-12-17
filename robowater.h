#ifndef __ROBOWATER__
#define __ROBOWATER__
#define F_CPU 3686400UL
#define BAUDRATE 115200UL
// Описание битов регистра "А" статуса и управления
#define RXC     7            // Флаг завершения приема USART
#define TXC     6            // Флаг завершения передачи USART
#define UDRE    5            // Флаг "Регистр данных USART пуст"
#define FE      4            // Флаг ошибки кадрирования
#define OVR     3            // Флаг переполнения
#define UPE     2            // Флаг ошибки контроля четности USART
#define MPCM    0            // Режим мультипроцессорного обмена
// Описание битов регистра "B" статуса и управления
#define RXCIE   7            // Разрешение прерывания по завершению приема
#define TXCIE   6            // Разрешение прерывания по завершению передачи
#define RXEN    4            // Разрешение приема
#define TXEN    3            // Разрешение передачи
#define UCSZ2   2            // Установка 9-ти битного режима
#define RXB8    1            // Разряд номер восемь приемного буфера
#define TXB8    0            // Разряд номер восемь буфера передачи.
// Описание статусов последовательных портов
#define FRAMING_ERROR	(1<<FE)
#define PARITY_ERROR    (1<<UPE)
#define DATA_OVERRUN    (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define TX_COMPLETE 	(1<<UDRE)
#define RX_COMPLETE 	(1<<RXC)
// maximum number of DS1820 devices connected to the 1 Wire bus
#define MAX_DS1820 4
#define FORCE_INIT 1
#define ALL_WARNINGS 12
#define ENTER_CANCEL_OVERFLOW 2      // Задается в кол-ве полсекундных интервалов
#define TIME_STOP 10                 // Время остановки Задается в кол-ве секундных интервалов
#define TIME_COOL_STOP 60            // Время таймаута для регулировки температуры калорифера в режиме СТОП
#define TIME_BUZ1 1                  // Время Звука 1
#define TIME_BUZ2 3                  // Время Звука 2
#define STRLENGTH 16                // Длина строки параметра
#define TIME_COOLING_MAX 15       //время задержки включения охладителя секунд
#define COUNT_FAN_MAX 5          // Максимальное количество осчетов для изменения оборотов вентилятора
#define POMP PORTD.5     // Порт.Пин насоса
#define COUNT_SIGNAL_MAX 50 // Количество тиков таймера 1 (1мс) для формирования сигналов звука и светодиодов. 0,5 сек / 1мс = 500
//#define COOLING1 PORTD.7     // Порт.Пин Охладителя 1
#define PWM_MIN prim_par.PWM1_lo     // Минимальное управляющее воздействие на двигатель
#define PWM_MAX prim_par.PWM1_hi     // Максимальное управляющее воздействие на двигатель П1=249 П2=245
#define TA_IN_NOLIMIT 500L      //500L Окончание ограничения температуры воздуха на входе (500 = +5 С)
#define TAP_ANGLE_LIMIT 78   //78.0 Ограничение закрытия трехходового крана при температуре воздуха TA_in_Min (ШИМ для 20% = 51: для 30% = 78))
//#define TAP_PWM_LIMIT 50   // Ограничение закрытия трехходового по напряжению снизу (ШИМ для 20% = 51: для 30% = 78))
#define MAX_ACCURACY 0b10//0b01       // Точность для термометров в бинарном коде: 0b11 = 0.0625, 0b10 = 0.125, 0b01 = 0.25, 0b00 = 0.5
#define MAX_OFFLINES 5
#define ENGINEERING 5
#define CHECK_EVENT (event == ev_none)
#define ds1820_devices prim_par.terms
// Чтение температуры (аргумент - номер термометра начиная с 0)
#define read_term(num) termometers[num].t
#define TIM1_COUNT 52767 // 61845
#define INITMODE 8                // Для запуска системы из меню 
#define T_TO 180                  // Время открытия - закрытия крана то
#define INIT_MODE 1
#define DUTY_MODE 0

// Описание типов переменных
typedef unsigned char 	byte;	// byte = unsigned char
typedef unsigned int 	word;	// word = unsigned int
typedef char str_val[STRLENGTH];
// Описание перечисления событий, возникающих в системе
enum en_event {
    ev_none = 0,            // [0]событие отсутствует
    ev_left = 1,            // [1]используется valcoder -
    ev_enter = 2,           // [2]полностью нажат/отжат enter
    ev_right = 3,           // [3]используется valcoder +
    ev_cancel,              // [4] полностью нажат/отжат cancel
    ev_secunda,             // [5] ежесекундное событие
    ev_start,               // [6] Нажатие кнопки ПУСК
    ev_stop,                // [7] Нажатие события СТОП
    ev_alarm1,              // [8]  \
    ev_alarm2,              // [9]  > События внешних тревог
    ev_filter,              // [10] /
    ev_freezing1,           // [11] Угроза замерзания по воздух вход
    ev_freezing2,           // [12] Угроза замерзания по Воздух выход
    ev_freezing3,           // [13] Угроза замерзания по воде выход
    ev_term1_nf,            // [14] Термометр 1 не найден
    ev_term2_nf,            // [15] Термометр 2 не найден
    ev_term3_nf,            // [16] Термометр 3 не найден
    ev_term4_nf,            // [17] Термометр 4 не найден
    ev_begin_to,            // [18] Пора проводить ТО
    ev_end_to,              // [19] Окончание ТО
    ev_to_nf                // [20] Невозможно провести ТО Крана
};
// перенес из boiler-control 15.05.2013
// Описание режимов работы системы
enum en_mode {
    mo_stop = 0,            //   0 - остановка в нормальном режиме,
    mo_action = 1,          //   1 - штатный режим с регулированием
    mo_to = 2,              //   2 - ТО крана (ADC1 + PWM1)
    mo_warming_up = 3,      //   3 - прогрев перед включением вентилятора,
    mo_warming_down = 4,    //   4 - охлаждение перед остановкой вентилятора,
    mo_setup_input1,        //   5 - установка ADC1
    mo_setup_input2,        //   6 - установка ADC2
    mo_setup_output1,       //   7 - настройка PWM1
    mo_setup_output2        //   8 - настройка PWM2,
};
// Описание cтруктур
struct st_datetime {
    byte cHH, cMM, cSS;         // Текущее время
    byte cyy, cmo, cdd;         // Текущая дата
    byte dayofweek;             // Текущий день недели (0..6), 0 - воскресенье, 1 - понедельник ...
};
// Структура проведения ТО
struct st_TO {
    byte weekday, hour, minute;
    byte status;                // Если ненулевое значение, то ТО сейчас проводится
};
// Структура основных переменных в системе
extern struct st_eeprom_par {
    byte tap_angle, fan_speed, ADC1, ADC2;        // 0x7F, 205, 0x7F, 0x7F, [4]
    int dt_winter, dt_summer;  
    int Ku, Ki, Kd;                             // 10, 0, 0 [6]
    int T_z, T_int;                             // Время интегртрования летом, Время интегрирования зимой
    byte T_start;                               // Время прогрева при старте системы. По умолчанию: 180 
    int TW_out_Min, TW_out_Stop;                // 1500, 5000,
    int TA_in_Min, TA_out_Min, TA_out_prs;    //-1500, 1000, 2200   [10]
    // перенес из boiler-control 15.05.2013
    byte PWM1_lo, PWM1_hi, ADC1_lo, ADC1_hi, PWM2_lo, PWM2_hi, ADC2_lo, ADC2_hi;    // Установка границ вольтажа входа и выхода
    byte warning_status[ALL_WARNINGS];           // 0) Тревога (0 - нет тревоги, > 0 - количество необработанных тревог) [12]
    byte season;          // Сезон года (?)      [1]
    byte alarm;           // Позиция текущего alarm в EEPROM [1]
    byte c_alarm;         // Сколько всего зарегистрировано в EEPROM [1]
    struct st_TO TO;     // Структура проведения ТО
    byte terms;           // Сколько должно быть термометров в системе
    byte addr[MAX_DS1820][9];  // [36] Найденные терм. (в адресе - 9 байт). Порядковый номер 0 - Помещение, 1 - Улица, 2 - вода вх., 3 - вода выход
} prim_par;

// Структура основных переключателей в системе
extern struct st_mode {
    byte menu;            // 0) Режим меню (00 - главный экран, 01 - меню, 10 - редактирование, 11 - сохраннение/восстановление параметра
    enum en_mode run;     // 1) Текущий режим работы (см. тип en_mode)
    byte initrun;         // 2) Запрашиваемы новый режим для запуска/остановки с меню
    byte pomp;            // 3) Режим работы насоса (1 = вкл, 0 выкл)
    byte fan;             // 4) Режим работы вентилятора (1 = вкл, 0 выкл)
    byte stop_sync_dt;    // 5) Флаг запрета синхронизации RTC и глобальной структуры. Если установлен - не синхронизируем
    byte sound;           // 6) Флаг запрета звука (0 = звук выключен, 1 = звук включен
    int k_angle_limit;   // 7) Коеффициент пересчета угла ограничения водяного крана в зимний период
    byte ufo[MAX_DS1820][9]; // 8) Массив Неопознанных термометров
    byte print;           // 9) Печать показаний системы регулирования
    byte cooling1;        // 10) Охладитель 1
    byte cooling2;        // 11) Охладитель 2
    byte new_terms;       // 12) Количество найденных новых термометров
    enum en_mode lastrun; // 13) Последний сохраненный режим перед переключением в новый       
} mode;
extern enum en_event event;        // Текущее событие в системе
// Описание функций
extern void init(void);
extern void set_cur_dt (void);
extern void get_cur_dt (unsigned char);
extern unsigned int read_adc(unsigned char adc_input);  // Read the AD conversion result
void switch_mode_run(enum en_mode newstatus);   // Переключение режимов в системе. Желательно пользоваться этой функцией
// Описание глобальных переменных
extern struct st_datetime s_dt;
extern struct st_mode mode;
extern unsigned int count_signal;
#endif
