#include <mega32.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include "robowater.h"
#include "fan.h"
#include "signals.h"
#include "season.h"
#include "menu.h"
#include "regular.h"
#include "pid.h"
// Описание макроподстановок
// Описание модульных переменных
//unsigned int time_integration = 0;
struct PID_DATA pidData;
// Подпрограмма регулирования охладителя (не зимой)
void coolant_regulator (void) {
    if (prim_par.season) return;   // защита от дурака-программиста
    // Для установок с водяным охладителем запускаем охладитель
    if (time_cooling == 0) {
        // if (POM_T > SET_T + prim_par.dt_summer) {
        if (UL_T > SET_T) {
            if (POM_T > SET_T) {
                /* if ( mode.cooling1 == 0) {
                      mode.cooling1 = 1;
                      signal_green(SHORT);
                } else {
                      mode.cooling2 = 1;
                      signal_green(LONG);
                } */
                if (mode.print) printf("Включен охладитель.  POM_T :%d\r\n",   POM_T);
            }
        }
        //Остановка охладителя
        if (POM_T < (SET_T - prim_par.dt_summer) || UL_T < SET_T) {
            /* if (mode.cooling1 == 1) {
                 mode.cooling1 = 0;
                 signal_green(SHORT);
            } else {
                 mode.cooling2 = 0;
                 signal_green(ON);
            } */
            if (mode.print) printf("Отключен охладитель. Разность температур - дельта: %d, POM_T :%d\r\n",  (SET_T - prim_par.dt_summer), POM_T);
        }
        time_cooling = prim_par.T_z;
        /* time_cooling = TIME_COOLING_MAX;
        mode.cooling1 = 0;
        mode.cooling2 = 0;
        count_cooling = 0; */
    }
}
// Подпрограмма поддержки работоспособности системы зимой в режиме останова
void keep_life_in_winter(void) {
    static int error_w_stop = 0;
    int water_out_bound;
    int water_out_bound_1;

    if (prim_par.season) tap_angle_check_range(DUTY_MODE); else return; // защита от дурака-программиста
    // Процесс поддержания температуры калорифера в режиме СТОП Зимой
    error_w_stop = (prim_par.TW_out_Stop - WOUT_T)/100;
    water_out_bound = prim_par.TW_out_Stop - 200;
    water_out_bound_1 = prim_par.TW_out_Stop - 500;
    // Разогрев калорифера
    if ((WOUT_T <= water_out_bound) && (timer_fan == 0)) {
        timer_fan = TIME_COOL_STOP;
        // forcheck_event = ev_pomp;
        TAP_ANGLE = TAP_ANGLE + error_w_stop;
        tap_angle_check_range(DUTY_MODE);
        if (WOUT_T < water_out_bound_1) {
            // if (mode.print) printf("Разогрев калорифера: %d, Угол крана расчетный :%d, Угол крана измеренный :%d, угол ограничения: %d, t обратки :%d  \r\n",  error_w_stop, TAP_ANGLE, ADC_VAR1, tap_angle_min, WOUT_T/10);
            signal_white(LONG);
            mode.pomp = 1;
        } else {
            signal_white(ON);
            mode.pomp = 0;
        }
        if (mode.print) printf("Разогрев калорифера: %d, Угол крана расчетный :%d, Угол крана измеренный :%d, t обратки :%d  \r\n",  error_w_stop, TAP_ANGLE, ADC_VAR1, WOUT_T/10);
    }
    // Охлаждение калорифера
    water_out_bound = prim_par.TW_out_Stop + 250;
    water_out_bound_1 = prim_par.TW_out_Stop + 500;
    if ((WOUT_T > water_out_bound) && (timer_fan == 0)) {
        timer_fan = TIME_COOL_STOP;
        tap_angle_check_range(DUTY_MODE);
        if ((WOUT_T >= water_out_bound) && (mode.pomp == 1)) {
            //if (mode.print) printf("Охлаждение калорифера: %d, Угол крана расчетный :%d, Угол крана измеренный :%d, t обратки :%d  \r\n",  error_w_stop, TAP_ANGLE, ADC_VAR1, WOUT_T/10);
            signal_white(ON);
            mode.pomp = 0;
        };
        if (WOUT_T >= water_out_bound_1)  TAP_ANGLE = TAP_ANGLE + error_w_stop;
        if (mode.print) printf("Охлаждение калорифера: %d, Угол крана расчетный :%d, Угол крана измеренный :%d, t обратки :%d  \r\n",  error_w_stop, TAP_ANGLE, ADC_VAR1, WOUT_T/10);
    }
    // Поддержание калорифера
    if (timer_fan == 0) {
        timer_fan = TIME_COOL_STOP;
        if (WIN_T >= prim_par.TW_out_Stop)  mode.pomp = 0;
        if (mode.print) printf("Поддержание калорифера: %d,  Угол крана расчетный :%d, Угол крана измеренный :%d, угол ограничения: %d, t обратки :%d  \r\n",  error_w_stop, TAP_ANGLE, ADC_VAR1, WOUT_T/10);
    }
}
// Подпрограмма регулирования калорифера зимой
void winter_regulator (void) {
    if (!prim_par.season) return;   // защита от дурака-программиста
    //  Вызываем функции регулирования только, если пришло время
    if (time_integration == 0) {
        prim_par.Ki = prim_par.Kd = 0;  // Заглушка, чтобы работала только функция update_P(). После отладки ПИД регулирования закомментировать эту строку. 
        if ((prim_par.Ki == 0) && (prim_par.Kd == 0)) {
            if (abs(SET_T - POM_T) > prim_par.dt_winter) {
                TAP_ANGLE += update_P(SET_T, POM_T, prim_par.Ku, prim_par.PWM1_hi - prim_par.PWM1_lo);
                printf("П регулирование. TAP_ANGLE = %d\r\n", TAP_ANGLE); 
            }
        } else {
            TAP_ANGLE += pid_Controller(SET_T, POM_T, &pidData);
            printf("ПИД регулирование. TAP_ANGLE = %d\r\n", TAP_ANGLE); 
        } 
        tap_angle_check_range(DUTY_MODE);
        time_integration = prim_par.T_int;
        // mode.cooling2 = mode.cooling1 = 0;
        time_cooling = 0;
        signal_green(ON);
    }
}
// Проверка TAP_ANGLE на принадлежность диапазону
void tap_angle_check_range(unsigned char check_mode) {
    int tmp_delta;
    unsigned char tap_angle_min = 0;   // Ограничение крана снизу от температуры
    tmp_delta = check_mode;             // Убираем предупреждения компилятора, если используем П-регулирование 
    #ifdef PID_H
    if (check_mode == INIT_MODE) pid_Init(prim_par.Ku * SCALING_FACTOR, prim_par.Ki * SCALING_FACTOR , prim_par.Kd * SCALING_FACTOR , &pidData); 
    #endif
    tmp_delta = abs(prim_par.TA_in_Min) + TA_IN_NOLIMIT;  // вычисление диапазона работы ограничителя крана по температуре
    mode.k_angle_limit = ((TAP_ANGLE_LIMIT * 1000) / tmp_delta); // вычисление коэффициента ограничения крана для заданных настроек   
    // Вычисление ограничения закрытия крана TAP_ANGLE = tap_angle_min
    if (UL_T < TA_IN_NOLIMIT) {
        // вычисление ограничения крана по температуре воздуха на входе и коэффициенту mode.k_angle_limit
        tap_angle_min = prim_par.PWM1_lo + ((long int)((TA_IN_NOLIMIT - UL_T) * mode.k_angle_limit)) / 1000;
        if (mode.print) printf(" tap_angle_min %u .   Kоэффициент :%d\r\n",  tap_angle_min, mode.k_angle_limit);
    }
    // Аппаратное ограничение закрытия крана по напряжению 2 вольта
    if (TAP_ANGLE <= prim_par.PWM1_lo) TAP_ANGLE = prim_par.PWM1_lo;
    if (mode.print) printf("Пересчет ограничения: %d, Ул. т :%d, Коэффициент :%i, ограничение снизу :%d, UL_T :%i  \r\n",  (TA_IN_NOLIMIT - UL_T), UL_T, mode.k_angle_limit, tap_angle_min, UL_T);
    if (TAP_ANGLE < tap_angle_min)
        TAP_ANGLE = tap_angle_min;
    else
        if (TAP_ANGLE >= prim_par.PWM1_hi) TAP_ANGLE = prim_par.PWM1_hi;
    if (mode.print) printf(" Установка крана %u .prim_par.PWM1_lo :%d\r\n", TAP_ANGLE,  prim_par.PWM1_lo);
}
