#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

#define N 10  // Número de leituras para média

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

uint8_t ssd[ssd1306_buffer_length];

void init_display() {
    // Inicializa o display OLED
    i2c_init(i2c1, 400000);  // I2C com clock de 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
}

float read_temperature() {
    const float VREF = 3.3f; // Tensão de referência
    adc_select_input(4);      // Seleciona o sensor de temperatura interno do RP2040
    adc_set_temp_sensor_enabled(true); // Ativa o sensor de temperatura

    // Leitura do ADC
    uint16_t raw = adc_read(); 

    if (raw < 100 || raw > 4000) {
        printf("Erro: Leitura do ADC fora do intervalo esperado!\n");
        return -1000; // Retorna um valor inválido em caso de erro
    }

    // Converte a leitura ADC para temperatura em ºC
    float voltage = (raw * VREF) / 4095;
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f; // Fórmula de calibração

    return temperature;
}

void update_display(float temperature) {
    // Limpa o buffer do display
    for (int i = 0; i < ssd1306_buffer_length; i++) ssd[i] = 0;

    // Exibe a temperatura no display como inteiro
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "Temp: %d C", (int)temperature);
    ssd1306_draw_string(ssd, 0, 8, temp_str);

    // Atualiza o display
    struct render_area display_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1,
    };
    calculate_render_area_buffer_length(&display_area);
    render_on_display(ssd, &display_area);
}

int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    adc_init();        // Inicializa o ADC

    // Inicializa o display OLED
    init_display();

    while (true) {
        float temperature = read_temperature();  // Lê a temperatura
        update_display(temperature);             // Atualiza o display com a temperatura lida
        sleep_ms(1000);  // Aguarda 1 segundo antes de ler novamente
    }

    return 0;
}
