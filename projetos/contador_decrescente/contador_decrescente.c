#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#include "ssd1306.h"
#include "ssd1306_i2c.h"

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15

#define BTN_A 5
#define BTN_B 6

volatile int contador = 9;
volatile int cliques = 0;
volatile bool tick = false;
volatile bool foi_resetado = false;
volatile bool atualiza_display = false;
volatile bool btn_b_debounce = false;
volatile absolute_time_t ultimo_clique_b;

repeating_timer_t timer;
uint8_t ssd[ssd1306_buffer_length];

struct render_area display_area = {
  .start_column = 0,
  .end_column = ssd1306_width - 1,
  .start_page = 0,
  .end_page = ssd1306_n_pages - 1,
};

bool on_timer(struct repeating_timer *t) {
  tick = true;
  return true;
}

void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == BTN_A && (events & GPIO_IRQ_EDGE_RISE)) {
      cliques = 0;  // Reseta os cliques apenas quando o contador está em 0
      contador = 9;  // Sempre reseta o contador regressivo
      foi_resetado = true;
      atualiza_display = true;
  }

  if (gpio == BTN_B && (events & GPIO_IRQ_EDGE_FALL)) {
    absolute_time_t agora = get_absolute_time();
    
    // Verifica se passou tempo suficiente desde o último clique e se o contador não está em 0
    if (absolute_time_diff_us(ultimo_clique_b, agora) > 500000 && contador > 0) { // 500ms
      cliques++;
      atualiza_display = true;
      ultimo_clique_b = agora;
    }
  }
}

void atualizar_display() {
  for (int i = 0; i < ssd1306_buffer_length; i++) ssd[i] = 0;

  char str[16];
  snprintf(str, sizeof(str), "Contador: %d", contador);
  ssd1306_draw_string(ssd, 0, 8, str);

  snprintf(str, sizeof(str), "Cliques: %d", cliques);
  ssd1306_draw_string(ssd, 0, 28, str);

  calculate_render_area_buffer_length(&display_area);
  render_on_display(ssd, &display_area);
}

int main() {
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_PIN);
  gpio_pull_up(SCL_PIN);
  sleep_ms(250);

  ssd1306_init();
  atualizar_display();

  // Configuração do botão A
  gpio_init(BTN_A);
  gpio_set_dir(BTN_A, GPIO_IN);
  gpio_pull_up(BTN_A);
  gpio_set_irq_enabled(BTN_A, GPIO_IRQ_EDGE_RISE, true);

  // Configuração do botão B
  gpio_init(BTN_B);
  gpio_set_dir(BTN_B, GPIO_IN);
  gpio_pull_up(BTN_B);
  gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);

  // Configurar callback para ambos os botões
  gpio_set_irq_callback(gpio_callback);
  irq_set_enabled(IO_IRQ_BANK0, true);

  add_repeating_timer_ms(-1000, on_timer, NULL, &timer);
  ultimo_clique_b = get_absolute_time();

  while (true) {
    if (tick) {
      tick = false;
      if (!foi_resetado && contador > 0) {  // Só decrementa se não estiver em 0
        contador--;
        atualiza_display = true;
      }
      foi_resetado = false;
    }

    if (atualiza_display) {
      atualizar_display();
      atualiza_display = false;
    }

    tight_loop_contents();
  }
}
