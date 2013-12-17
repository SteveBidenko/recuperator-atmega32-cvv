#include <stdio.h>
#include <string.h>
#include "robowater.h"
#include "at2404.h"
#include "spd1820.h"
#include "alarm.h"
#include "bits.h"
#include "menu.h"

struct st_alarm alarm;      // Текущий alarm

// Если не предупреждение, то эти все тревоги могу стать аварией
flash alarm_text all_warnings[MAX_WARNINGS] = {
        "Термозащита ",  // [0] срабатывание внешнего датчика перегрева вентилятора или пожар. Для старта мы должны дать 30 сек для выхода в рабочее состояние частотного преобразователя (может аварией)
        "Замерзание ",   // [1] срабатывание внешнего датчика температуры калорифера (обычно настраивают +10)
        "Темп.возд.вх ", // [2] результаты замеров и мат.обработки
        "Т.возд.вых. ",  // [3] результаты замеров и мат.обработки
        "Темп.воды вх ", // [4] результаты замеров и мат.обработки
        "Т.воды вых.",   // [5] результаты замеров и мат.обработки
        "Нет ТО ",       // [6] результат выполнения сценария с проверкой внешнего датчика (Предупреждение)
        "Нет терм.TE1 ",  // [7] проблема с термометром на выходе воздуха (помещение)
        "Нет терм.TE2 ",  // [8] проблема с термометром на входе воздуха (улица)
        "Нет терм.TE3 ",  // [9] проблема с термометром на входе воды. (предупреждение)
        "Нет терм.TE4 ",  // [10] проблема с термометром на выходе воды.
        "Фильтр "        // [11] срабатывание внешнего датчика, т.е. проблема с воздушным Фильтром. (предупреждение)
};
// Функция обновляющая количество активных warnings в системе для меню
void update_warning_menu (void) {
    register unsigned char i;
    SUM_ALARMS = 0;
    for (i = 0; i < MAX_WARNINGS; i++) {
        warnings[i].can_edit = prim_par.warning_status[i];
        if (prim_par.warning_status[i]) {
            SUM_ALARMS++;
            if (warnings[i].val_data == 0) warnings[i].val_data = 1;
        }
    }
    IS_ALARM = SUM_ALARMS;
}
// Функция регистрации ALARM
void alarm_reg (int reg_preset, int reg_alarm, char *text_alarm, unsigned char code) {
    sprintf (alarm.datestamp, "%02u:%02u %02u.%02u", s_dt.cHH, s_dt.cMM, s_dt.cdd, s_dt.cmo);
    sprintf (alarm.text, "%s", text_alarm);
    alarm.val_preset = reg_preset;
    alarm.val_alarm = reg_alarm;
    // Регистрируем событие в общей структуре prim_par
    prim_par.warning_status[code] = 1;
    warnings[code].val_data = reg_alarm;
    // Вычисление позиции необработанного alarm
    if (prim_par.c_alarm < MAX_ALARMS) prim_par.c_alarm++;
    prim_par.alarm++;
    if (prim_par.alarm > prim_par.c_alarm) prim_par.alarm = 1;
    // Запись ALARM
    eeprom_write_alarm ((char *)&alarm, sizeof(alarm), prim_par.alarm);
    // Запись структуры установок (там хранится позиция и номер ALARM)
    eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
    update_warning_menu ();
    return;
}
// Функция обработки ALARM
signed char alarm_unreg (unsigned char code) {
    // Если событий нет, возвращаем признак -1
    if (prim_par.warning_status[code] == 0) return -1;
    // Разрегистрируем событие в общей структуре prim_par
    prim_par.warning_status[code] = 0;
    // Запись структуры установок (там хранится позиция и номер ALARM)
    eeprom_write_struct ((char *)&prim_par, sizeof(prim_par));
    update_warning_menu ();
    if (code >= 7 && code <= 10) read_all_terms(INIT_MODE);
    return;
}
// Функция чтения ALARM по номеру из EEPROM
signed char alarm_read (unsigned char num) {
    // Если событий нет, возвращаем признак -1
    if (num == 0) return -1;
    eeprom_read_alarm ((char *)&alarm, sizeof(alarm), num);
}
void alarm_c_print (void) {
    printf("> %s %i %i %s\r\n", alarm.datestamp, alarm.val_preset, alarm.val_alarm, alarm.text);
}
// Функция печатающая все ALARMs
void alarm_all_print (void) {
    register unsigned char i;
    unsigned char c;
    c = prim_par.alarm;
    // Печатаем все зарегистрированные alarms начиная с текущей и в обратном порядке
    printf("Все аварии:\r\n");
    for (i = 1; i <= MAX_ALARMS; i++) {
        alarm_read(c--);
        alarm_c_print();
        if (c == 0) c = prim_par.c_alarm;
    }
}
// Функция, возвращающая строку названия тревоги по коду
char *get_warning_str(unsigned char code) {
    static alarm_text warning_string;
    if (code < MAX_WARNINGS) {
        strcpyf (warning_string, all_warnings[code]);
        return warning_string;
    } else {
        return NULL;
    }
}