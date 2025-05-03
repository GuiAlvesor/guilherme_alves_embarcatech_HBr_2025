#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "../drivers//adc_to_celsius.h"


int main()
{
    stdio_init_all();
    adc_init();
    adc_select_input(4);
   
    while (true) {
        uint16_t value = adc_read();
        adc_to_celsius(value);
        float temp = adc_to_celsius(value);
        sleep_ms(1000);
        printf("%.2f\n", temp);
    }
}


