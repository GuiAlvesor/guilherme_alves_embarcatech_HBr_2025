// temp_interna_display.c – Lê temperatura interna do RP2040 e exibe no OLED

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define I2C_SDA 0
#define I2C_SCL 1

ssd1306_t display;

void init_display() {
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&display, 128, 64, i2c0, 0x3C, false);
    ssd1306_clear(&display);
}

float read_internal_temperature() {
    adc_select_input(4); // canal interno para temperatura
    uint16_t raw = adc_read();
    const float conversion_factor = 3.3f / (1 << 12); // 12 bits
    float voltage = raw * conversion_factor;
    float temp_c = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temp_c;
}

int main() {
    stdio_init_all();
    adc_init();
    adc_set_temp_sensor_enabled(true);

    init_display();

    while (true) {
        float temp = read_internal_temperature();

        char buffer[32];
        sprintf(buffer, "Temp interna: %.2f C", temp);

        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, 1, buffer);
        ssd1306_show(&display);

        sleep_ms(1000);
    }
}

