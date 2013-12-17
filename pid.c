#include "pid.h"

// Функция пропорционального регулирования
signed short update_P(signed short setPoint, signed short processValue, unsigned char p_factor, unsigned int range) {
    int error;
    long int coefficient = 0;
    signed short ret; 

    error = setPoint - processValue;
    coefficient = (long int) range * p_factor / 1000;
    ret = coefficient * (error / 100) ;
    // printf("Разность температур: %d, coefficient: %d, воздействие: %d\r\n", error, coefficient, ret);
    return (ret);
}
// Set up PID controller parameters
void pid_Init(signed short p_factor, signed short i_factor, signed short d_factor, struct PID_DATA *pid) {
    pid->lastProcessValue = pid->sumError = 0;    // Start values for PID controller
    // Tuning constants for PID loop
    pid->P_Factor = p_factor;
    pid->I_Factor = i_factor;
    pid->D_Factor = d_factor;
    // Limits to avoid overflow
    pid->maxError = MAX_INT / (pid->P_Factor + 1);
    pid->maxSumError = MAX_I_TERM / ((long)pid->I_Factor + 1);
}
// Calculates output from setpoint, process value and PID status.
signed short pid_Controller(signed short setPoint, signed short processValue, struct PID_DATA *pid) {
    signed short error, p_term, d_term;
    signed long i_term, ret, temp;

    error = setPoint - processValue;
    // Calculate Pterm and limit error overflow
    if (error > pid->maxError) {
        p_term = MAX_INT;
    } else if (error < -pid->maxError) {
        p_term = -MAX_INT;
    } else {
        p_term = pid->P_Factor * error;
    }

    // Calculate Iterm and limit integral runaway
    temp = pid->sumError + error;
    if (temp > pid->maxSumError) {
        i_term = MAX_I_TERM;
        pid->sumError = pid->maxSumError;
    } else if (temp < -pid->maxSumError) {
        i_term = -MAX_I_TERM;
        pid->sumError = -pid->maxSumError;
    } else {
        pid->sumError = temp;
        i_term = pid->I_Factor * pid->sumError;
    }

    // Calculate Dterm
    d_term = pid->D_Factor * (pid->lastProcessValue - processValue);
    pid->lastProcessValue = processValue;
    ret = (p_term + i_term + d_term) / SCALING_FACTOR;
    if (ret > MAX_INT) {
        ret = MAX_INT;
    } else if (ret < -MAX_INT) {
        ret = -MAX_INT;
    }
    
    return((signed short)ret);
}
// Calling this function will reset the integrator in the PID regulator.
// Надо понять, где должна вызываться эта функция
/*
void pid_Reset_Integrator(struct PID_DATA *pid) {
    pid->sumError = 0;    
}
*/

