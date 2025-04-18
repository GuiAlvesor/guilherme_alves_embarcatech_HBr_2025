// joystick_read.c – Lê valores analógicos do joystick com eixos X e Y invertidos

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define JOY_X 26 // GPIO26 -> ADC0
#define JOY_Y 27 // GPIO27 -> ADC1

void init_joystick_adc() {
    adc_init();
    adc_gpio_init(JOY_X); // canal 0
    adc_gpio_init(JOY_Y); // canal 1
}

uint16_t read_axis(uint input) {
    adc_select_input(input);
    return adc_read(); // 0 a 4095
}

int main() {
    stdio_init_all();
    init_joystick_adc();

    while (true) {
        uint16_t y = read_axis(0); // ADC0 – valor físico em X, representando Y
        uint16_t x = read_axis(1); // ADC1 – valor físico em Y, representando X

        printf("Joystick X: %u, Y: %u\n", x, y);
        sleep_ms(200);
    }
}

