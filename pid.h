#ifndef PID_H
#define PID_H
// Объявление макроподстановок
#define SCALING_FACTOR  128
#define MAX_INT         32767
#define MAX_LONG        2147483647
#define MAX_I_TERM      (MAX_LONG / 2)
// Объявление типов
typedef struct PID_DATA{
  signed short lastProcessValue;// Last process value, used to find derivative of process value.
  signed long sumError;         // Summation of errors, used for integrate calculations
  signed short P_Factor;        // The Proportional tuning constant, multiplied with SCALING_FACTOR
  signed short I_Factor;        // The Integral tuning constant, multiplied with SCALING_FACTOR
  signed short D_Factor;        // The Derivative tuning constant, multiplied with SCALING_FACTOR
  signed short maxError;        // Maximum allowed error, avoid overflow
  signed long maxSumError;      // Maximum allowed sumerror, avoid overflow
} pidData_t;
// Объявление внешних данных
// Объявление функций 
void pid_Init(signed short p_factor, signed short i_factor, signed short d_factor, struct PID_DATA *pid);
signed short update_P(signed short setPoint, signed short processValue, unsigned char p_factor, unsigned int range);
signed short pid_Controller(signed short setPoint, signed short processValue, struct PID_DATA *pid); // pid was pid_st
// void pid_Reset_Integrator(struct PID_DATA *pid); // pid was pid_st
#endif
