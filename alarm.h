#ifndef __ALARMS__
#define __ALARMS__

// ALARM - зарегистрированное событие, строка в журнале
// WARNING - тревога, обозначает неисправность или проблему конкретного оборудования
#define MAX_WARNINGS 12        // Необходимо синхронизировать вручную с robowater.h #define ALL_WARNINGS
#define DT_LENGTH 12
#define ALARM_LENGTH 16      // Максимальная длина строки ALARM
#define MAX_ALARMS 8

typedef char alarm_text[ALARM_LENGTH];
// Описание структуры ALARM
struct st_alarm {
    char datestamp[DT_LENGTH];  // Отпечаток даты/времени ALARM
    int val_preset;             // Заданное значение
    int val_alarm;              // Значение ALARM
    alarm_text text;            // Текстовая расшифровка ALARM
};
// Описание внешних функций
extern void alarm_reg (int reg_preset, int reg_alarm, char *text_alarm, unsigned char);
extern signed char alarm_unreg (unsigned char);
extern void alarm_all_print (void);
extern char *get_warning_str(unsigned char);
extern void update_warning_menu (void);

#endif
