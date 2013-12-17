#include <mega32.h>
#include "robowater.h"
#include "valcoder.h"

// byte valcoder0, valcoder1;      // Биты прерываний
// word counter0, counter1;        // счетчик кол-ва прерываний от valcoder
signed char valcoder;              // Направление вращения
unsigned char EncState; // было прерывание вращения INT0
unsigned char EncReady; // Энкодер обработан
// Внешние прерывания Valcoder'а
interrupt [EXT_INT0] void ext_int0_isr(void) {
    valcoder = 0;
    // Устраняем дребезг прерывания
    GIFR |= (1<<6);
    // Запрещаем прерывание INT0 (выставляем 0 в 6 бите)
    GICR &= ~(1<<6);
    EncState = 1;
}
interrupt [EXT_INT1] void ext_int1_isr(void) {
    GIFR |= (1<<7);
    // Разрешаем прерывание INT0 (выставляем 1 в 6 бите)
    if ( EncReady == 1 ) {
        EncReady = 0;
        GICR |= (1<<6);
    };
}
