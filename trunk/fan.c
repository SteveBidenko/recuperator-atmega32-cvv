#include <stdio.h>
#include "robowater.h"
#include "menu.h"
#include "fan.h"
unsigned char timer_fan = 0;
unsigned char count_fan = 0;
// Функция возвращающая заданный аргумент только, если значение лежит в диапазоне между FAN_SPEED_MIN и FAN_SPEED_MAX.
unsigned char check_fan_range(int value) {
    unsigned char new_value;
    if (value <= FAN_SPEED_MIN) {
        new_value = FAN_SPEED_MIN;
    } else {
        new_value = (value > FAN_SPEED_MAX) ? FAN_SPEED_MAX : value; 
    } 
    return (new_value); 
}
// Подпрограмма регулирования скорости вентилятора зимой
void winter_fan_speed(void) {
    if ((TAP_ANGLE == PWM_MAX) && ((POM_T - FAN_SPEED_T_DOWN) < SET_T)) {
        count_fan++;
        if (count_fan == COUNT_FAN_MAX) {
           FAN_SPEED = check_fan_range(FAN_SPEED - FAN_SPEED_STEP);
           count_fan = 0;
        if (mode.print) printf("Понижение скорости вентилятора расчетное : %d, измеренное : %d, Счетчик циклов :%d, POM_T :%d \r\n",  FAN_SPEED, ADC_VAR2, count_fan, POM_T);
        }
    } else  {// count_fan = 0;
        if ((FAN_SPEED < prim_par.fan_speed) && ((POM_T + FAN_SPEED_T_UP) > SET_T)) {
            count_fan++;
            if (count_fan == COUNT_FAN_MAX) {
               FAN_SPEED = check_fan_range(FAN_SPEED + FAN_SPEED_STEP);
               count_fan = 0;
               if (mode.print) printf("Увеличение скорости вентилятора расчетная: %d, измеренная : %d, Счетчик циклов :%d, Заданная скорость :%d \r\n",  FAN_SPEED, ADC_VAR2, count_fan, prim_par.fan_speed);
            }

        } else count_fan = 0;
    }
    if (mode.print) printf("Скорость вентилятора расчетная: %d, измеренная : %d, Заданная скорость: %d, POM_T: %d\r\n",  FAN_SPEED, ADC_VAR2, prim_par.fan_speed, POM_T);
}
