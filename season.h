#ifndef _SEASON_
#define _SEASON_
// Определение подстановок

// Описание функций
extern void coolant_regulator(void);   // Подпрограмма регулирования охладителя (не зимой)
extern void keep_life_in_winter(void); // поддержка работоспособности системы зимой
extern void winter_regulator(void);    // Подпрограмма регулирования калорифера зимой
// Описание глобальных переменных
//extern unsigned int time_integration;
void tap_angle_check_range(unsigned char check_mode); // Проверка TAP_ANGLE на принадлежность диапазону с учетом уличной температуры. Если check_mode == INIT_MODE, инициализируем PID
#endif
