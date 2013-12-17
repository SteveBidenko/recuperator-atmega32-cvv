#ifndef _VALCODER_
#define _VALCODER_
// ќпределение подстановок
#define VALCODER_PIN0 (PIND.2)
#define VALCODER_PIN1 (PIND.3)
#define VALCODER_DISABLE() GICR &= ~(3<<6)
#define VALCODER_ENABLE() GICR |= (3<<6)
#define VALCODER_NO_ROTATE 0
#define VALCODER_DELAY 15
#define VALCODER_SENSITY 1
// ќписание функций
// ќписание глобальных переменных
extern signed char valcoder;
extern unsigned char EncState; // было прерывание вращени€ INT0
extern unsigned char EncReady; // Ёнкодер обработан
#endif
