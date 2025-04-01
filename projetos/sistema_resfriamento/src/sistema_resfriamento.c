#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#define BUZZER_PIN 21
#define BUZZER_PIN_2 22
#define BUZZER_FREQUENCY 100
#define TEMPERATURE_LIMIT 40
#define N 10 

#define MOTOR_PWM 8   // Pino PWM para controle da velocidade (EN)
#define MOTOR_IN1 9   // Pino de controle de direção (IN1)

#define BUTTON_INCREMENT 6  
#define BUTTON_DECREMENT 5  

// Coeficientes de regressão linear (baseados em calibração)
#define CALIB_M 1.02f  // Inclinação da correção
#define CALIB_B -0.75f // Ajuste de offset

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

int numero = 0;

void pwm_init_buzzer(uint pin);
void beep(uint pin, uint duration_ms);
float refined_temperature();
void play_beep(uint pin1, uint pin2);

#include <stdio.h>

#define NOTE_Eb4 311
#define NOTE_D4  293
#define NOTE_Gm  196  

int melody[] = {
    NOTE_Gm, NOTE_Gm, NOTE_Gm, 
    NOTE_Gm, NOTE_Gm, NOTE_Gm,
};

int noteDurations[] = {
    600, 600, 600,
    600, 600, 600, 
};


void set_motor_speed(int speed) {
    uint slice_num = pwm_gpio_to_slice_num(MOTOR_PWM); //slice 4
    uint wrap = pwm_hw->slice[slice_num].top;

    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;

    uint16_t duty = (speed * wrap) / 100;  // Converte 0-100% para PWM
    pwm_set_gpio_level(MOTOR_PWM, duty);  // Controla a velocidade

    // Direção fixa (motor sempre gira no mesmo sentido)
    gpio_put(MOTOR_IN1, 1);  // Gira sempre no sentido horário
}

int value = 40;  

void button_isr(uint gpio, uint32_t events) {
    if (gpio == BUTTON_INCREMENT) {
        value++;  
        printf("%d", value);
    } else if (gpio == BUTTON_DECREMENT) {
        value--;  
        printf("%d", value);
    }
}

int main()
{
    stdio_init_all();   
    adc_init(); 
    pwm_init_buzzer(BUZZER_PIN);

    gpio_init(BUTTON_INCREMENT);
    gpio_init(BUTTON_DECREMENT);
    
    gpio_set_dir(BUTTON_INCREMENT, GPIO_IN);   
    gpio_set_dir(BUTTON_DECREMENT, GPIO_IN);   
    
    gpio_pull_up(BUTTON_INCREMENT);
    gpio_pull_up(BUTTON_DECREMENT);

    // Configura as interrupções
    gpio_set_irq_enabled_with_callback(BUTTON_INCREMENT, GPIO_IRQ_EDGE_RISE, true, button_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_DECREMENT, GPIO_IRQ_EDGE_RISE, true, button_isr);

    gpio_set_function(MOTOR_PWM, GPIO_FUNC_PWM); // Configura PWM no EN
    gpio_init(MOTOR_IN1);
    gpio_set_dir(MOTOR_IN1, GPIO_OUT);  // Controle de direção

    uint slice_num = pwm_gpio_to_slice_num(MOTOR_PWM);
    pwm_set_wrap(slice_num, 10000);  // Define o período de PWM (20ms)
    pwm_set_clkdiv(slice_num, 64.0); // Ajusta a frequência do PWM
    pwm_set_enabled(slice_num, true);

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
    pwm_init_buzzer(BUZZER_PIN);
    pwm_init_buzzer(BUZZER_PIN_2);

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    while(true) {
        char buffer[20];
        float temperatura = refined_temperature(); 
        memset(ssd, 0, ssd1306_buffer_length);
        render_on_display(ssd, &frame_area);
        
        sprintf(buffer, "Set:%d", value);
        ssd1306_draw_string(ssd, 32, 16, buffer);
        
        sprintf(buffer, "Temp:%d;C", (int)temperatura);
        ssd1306_draw_string(ssd, 32, 32, buffer);
        
        render_on_display(ssd, &frame_area);

        if (temperatura >= value) {
            memset(ssd, 0, ssd1306_buffer_length);
            render_on_display(ssd, &frame_area);
            sprintf(buffer, "Set:%d", value);
            ssd1306_draw_string(ssd, 32, 16, buffer);
            sprintf(buffer, "OVERHEAT! %d;C", (int)temperatura);
            ssd1306_draw_string(ssd, 16, 32, buffer);
            render_on_display(ssd, &frame_area);
            set_motor_speed(50);
            play_beep(BUZZER_PIN, BUZZER_PIN_2);
        }else{
            set_motor_speed(100); //desliga o motor                                                     
        }
        sleep_ms(1000);
    }

    return 0;
}

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys)/(BUZZER_FREQUENCY * 4090));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
} 

void play_tone(uint pin, uint frequency, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t wrap_value = 125000000 / (frequency * 4);
    pwm_set_wrap(slice_num, wrap_value);
    pwm_set_gpio_level(pin, wrap_value / 2);
    sleep_ms(duration_ms);
    pwm_set_gpio_level(pin, 0);
}

void play_beep(uint pin1, uint pin2) {
    for(int i=0; i< sizeof(melody)/sizeof(melody[0]); i++) {
       if(melody[i] == 0) {
           sleep_ms(noteDurations[i]);
       } else {
           play_tone(pin1, melody[i], noteDurations[i]);
           play_tone(pin2, melody[i], noteDurations[i]);
           sleep_ms(noteDurations[i]);
       }
    }
}

float refined_temperature() {
    const float VREF = 3.3f; 
    adc_set_temp_sensor_enabled(true); 
    adc_select_input(4); 
    adc_set_clkdiv(0);

    float readings[N];
    float sum = 0, mean, stddev = 0;

    // 🔹 1º Algoritmo: Média de múltiplas leituras
    for (int i = 0; i < N; i++) {
        uint16_t raw = adc_read(); 
        if (raw < 100 || raw > 4000) {
            printf("Erro: Leitura do ADC fora do intervalo esperado!\n");
            return -1000; 
        }

        float voltage = (raw * VREF) / 4095; 
        

        readings[i] = 27.0f - (voltage - 0.706f) / 0.001721f; 
        sum += readings[i];

        sleep_ms(50); 
    }

    mean = sum / N;

    // 🔹 2º Algoritmo: Ajuste por regressão linear (calibração personalizada)
    mean = (CALIB_M * mean) + CALIB_B; 

    // 🔹 3º Algoritmo: Filtragem de outliers com desvio padrão
    for (int i = 0; i < N; i++) {
        stddev += (readings[i] - mean) * (readings[i] - mean);
    }
    stddev = sqrt(stddev / N);

    sum = 0;
    int valid_count = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(readings[i] - mean) <= (2 * stddev)) { 
            sum += readings[i];
            valid_count++;
        }
    }

    float refined_temperature = (valid_count > 0) ? sum / valid_count : mean; 
    refined_temperature = trunc(refined_temperature);
    printf("Temp: %.2f\n", refined_temperature);
    return refined_temperature; 
}