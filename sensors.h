#ifndef _SENSORS_
#define _SENSORS_
// для совместимости
#define KEY_STOP  key(0)    // [0] KEY_STOP   PINC.3
//#define KEY_STOP_RANGE  range(0)   // [0] KEY_STOP   выборы
#define KEY_START key(1)    // [1] KEY_START  PINC.5
#define KEY_ENTER key(2)    // [5] KEY_ENTER PINC.6
#define KEY_CANCEL key(3)   // [6] KEY_CANCEL PINC.7
#define SENSOR_ALARM1 key(4)   // [2] SENSOR_ALARM1 PINC.2
#define SENSOR_ALARM2 key(5)   // [3] SENSOR_ALARM2 PINC.4
#define SENSOR_FILTER key(6)   // [4] SENSOR_FILTER PINB.1
// Конечное значение выборов
#define MAX_POLL 100
// Возможные статусы
#define KEY_ON 0            // Здесь устанавливается уровень сигнала
#define KEY_OFF 1           // Здесь устанавливается уровень сигнала
#define KEY_POLL -1         //
#define KEY_INACTIVE 10     // Таймер, определяющий работоспособность кнопки
// Кол-во кнопок
#define NUM_KEYS 7
// Глобальная переменная
extern unsigned char t_key; // Для таймера
extern unsigned char key_treated[7] ;
// Описание функций
void init_keys(void);       // Инициализация всех кнопок
void poll_keys(void);       // Опрос всех кнопок
void poll_key(unsigned char); // Опрос конкретной кнопки
unsigned char key(unsigned char);
#endif
