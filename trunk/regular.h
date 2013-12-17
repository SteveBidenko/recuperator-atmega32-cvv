#ifndef _REGULAR_
#define _REGULAR_
// Определение подстановок

// Описание функций
extern void regular_check_alarm_and_warnings(void); // Функция, регулярно проверяющая наличие warnings and alarms
extern void regular_inspection(void);   // Функция, регулярно запускающая по событию Секунда. Вызов функции - не гарантирован.
extern void lcd_primary_screen(void);
extern char high_time_TO(void);        // Функция, проверяющая необходимость проведения ТО
extern void link_terms(void);       // Функция связывания значения термометров с глобальными переменными
// Описание глобальных переменных
extern unsigned int time_integration;
extern unsigned int timer_start;
extern byte timer_stop;
extern int time_cooling;
extern unsigned char timer_start_to;
extern unsigned char timer_stop_to;
#endif
