#include "../../process.h"

void timer_init(void) {
    // Setup SysTick Timer for 1ms interrupts
    SysTick_Config(SystemCoreClock / 1000);
}

void SysTick_Handler(void) {
    timer_handler();
}
