#ifndef _FAN_
#define _FAN_
#include "robowater.h"
// Определение подстановок
#define FAN_SPEED_MIN prim_par.PWM2_lo     // Минимальный предел оборотов вентилятора 
#define FAN_SPEED_MAX prim_par.PWM2_hi     // Мaксимальный предел оборотов вентилятора 
#define FAN_SPEED_STEP 10            // Шаг изменения оборотов вентилятора
#define FAN_SPEED_T_UP 300
#define FAN_SPEED_T_DOWN 200
#define FAN PORTD.4    // Порт.Пин вентилятора
// Описание функций
extern unsigned char check_fan_range(int value);  
extern void winter_fan_speed(void);    // регулирование вентилятора зимой
// Описание глобальных переменных
extern byte timer_fan;
extern byte count_fan;
#endif

