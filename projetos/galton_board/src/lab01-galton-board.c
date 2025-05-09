#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "inc/ssd1306.h"


// ---------------------- CONFIGURAÇÕES DO DISPLAY E PINOS ------------------------
#define PINO_SDA 14                         // Pino SDA (I2C)
#define PINO_SCL 15                         // Pino SCL (I2C)
#define TAMANHO_BUFFER (ssd1306_width * ssd1306_n_pages) // Tamanho do buffer do display
#define BOTAO_A 5
#define BOTAO_B 6
// ---------------------- PARÂMETROS VISUAIS DA ESTRUTURA --------------------------
#define ESPACO_ENTRE_PINOS 4                // Espaço entre os pinos
#define NUM_LINHAS 10                       // Número de linhas de pinos
#define Y_INICIAL 5                         // Altura inicial da estrutura
#define DESLOCAMENTO 2                      // Passo do movimento da bola
#define X_BASE ((ssd1306_width / 2) - ((NUM_LINHAS / 2) * ESPACO_ENTRE_PINOS)) // Posição horizontal base dos pinos

#define NUM_MAX_BOLAS 99                    // Número máximo de bolas simultâneas

volatile bool desbalanceamento1 = false;
volatile bool desbalanceamento2 = false;
// ---------------------- ESTRUTURA DA BOLA --------------------------
struct Bola {
    uint8_t x, y;  // Posição atual da bola no display
    bool ativa;    // Status de atividade (true = em movimento, false = inativa)
};

// ---------------------- INICIALIZA GERADOR ALEATÓRIO COM ADC --------------------------
void iniciar_aleatorio() {
    adc_init();
    adc_select_input(0);                    // Seleciona canal ADC 0
    uint16_t ruido = adc_read();            // Lê ruído elétrico como semente pseudoaleatória
    srand(ruido);                           // Inicializa o rand() com essa semente
}

// ---------------------- CONFIGURA I2C E DISPLAY SSD1306 --------------------------
void iniciar_i2c() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PINO_SDA);
    gpio_pull_up(PINO_SCL);
    ssd1306_init();                         // Inicializa o display
}

// ---------------------- DESENHA A ESTRUTURA DE PINOS --------------------------
void desenhar_pinos(uint8_t *tela, struct render_area *area_render, int8_t num_linhas) {
    for (int linha = 0; linha < num_linhas; linha++) {
        int num_pinos = linha + 1;                                 // Pinos aumentam por linha
        int y = (linha * ESPACO_ENTRE_PINOS) + Y_INICIAL;
        int x_inicio = (ssd1306_width / 2) - 2 * (num_pinos - 1);

        for (int i = 0; i < num_pinos; i++) {
            int x = x_inicio + i * ESPACO_ENTRE_PINOS;
            ssd1306_set_pixel(tela, x, y, 1);                       // Desenha um pino
        }
    }
}

// ---------------------- ESCOLHA ALEATÓRIA: ESQUERDA OU DIREITA --------------------------
bool direcao_aleatoria() {
    return rand() % 2;
}

// ---------------------- MOVIMENTO DA BOLA E ATUALIZA HISTOGRAMA --------------------------
void atualizar_bola(struct Bola *bola, uint8_t *histograma) {
    if (((bola->y - Y_INICIAL) % 4 <= 1) && (bola->y >= 4) && (bola->y <= (NUM_LINHAS * ESPACO_ENTRE_PINOS))) {
        int direcao = direcao_aleatoria()? 1 : -1;
        if (desbalanceamento1 == true && direcao == 1) {
            direcao *= -1;
        } 
        if (desbalanceamento2 == true && direcao == -1 && rand()%2 == 0) {
            direcao *= -1;
        }
        bola->x += DESLOCAMENTO * direcao;
    }

    bola->y += DESLOCAMENTO; // Move a bola para baixo

    if (bola->y > 50) {
        int indice = (bola->x - X_BASE) / ESPACO_ENTRE_PINOS;
        if (indice >= 0 && indice <= NUM_LINHAS) {
            histograma[indice]++;  // Soma 1 à barra correspondente
        }
        bola->ativa = false;       // Desativa a bola
    }
}

// ---------------------- DESENHA BOLA ATIVA --------------------------
void desenhar_bola(uint8_t *tela, struct Bola *bola) {
    ssd1306_set_pixel(tela, bola->x, bola->y, 1);
}

// ---------------------- DESENHA HISTOGRAMA --------------------------
void atualizar_histograma(uint8_t *tela, uint8_t *histograma) {
    for (int i = 0; i <= NUM_LINHAS; i++) {
        if (histograma[i] > 0) {
            int x = X_BASE + i * ESPACO_ENTRE_PINOS;
            int y_base = ssd1306_height - 1;

            for (int y = y_base; y > y_base - histograma[i]; y--) {
                if (y >= 0) {
                    ssd1306_set_pixel(tela, x, y, 1); // Desenha barra
                }
            }
        }
    }
}

// ---------------------- INSERE UMA NOVA BOLA --------------------------
void gerar_bola(struct Bola *bolas) {
    for (int i = 0; i < NUM_MAX_BOLAS; i++) {
        if (!bolas[i].ativa) {
            bolas[i].x = ssd1306_width / 2;
            bolas[i].y = 0;
            bolas[i].ativa = true;
            return;
        }
    }
}

void desbalanceamento_experimental(uint gpio, uint32_t events) {
    if(gpio == BOTAO_A) {
        desbalanceamento1 = true;
        desbalanceamento2 = false;
    }
        if(gpio ==BOTAO_B) {
        desbalanceamento2 = true;
        desbalanceamento1 = false;
        }
}

void iniciar_botao_com_callback() {
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled(BOTAO_A, GPIO_IRQ_EDGE_FALL, true);  // Quando botão for pressionado
    
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);  // Quando botão for pressionado
    gpio_set_irq_callback(desbalanceamento_experimental);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

// ---------------------- FUNÇÃO PRINCIPAL --------------------------
int main() {
    stdio_init_all();       // Inicializa USB serial
    iniciar_i2c();          // Inicializa display
    iniciar_aleatorio();    // Inicializa rand()
    iniciar_botao_com_callback();
    struct render_area area_render = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&area_render);
    uint8_t tela[TAMANHO_BUFFER]; // Buffer da tela
    memset(tela, 0, TAMANHO_BUFFER);
    render_on_display(tela, &area_render); // Limpa tela

    struct Bola bolas[NUM_MAX_BOLAS];
    for (int i = 0; i < NUM_MAX_BOLAS; i++) bolas[i].ativa = false;

    uint8_t histograma[NUM_LINHAS + 1] = {0};
    int contador_tempo = 0;
    uint8_t contador_bolas = 0;
    char texto_esquerda[100];
    char texto_direita[10];
    while (true) {
        memset(tela, 0, TAMANHO_BUFFER); // Limpa buffer
        snprintf(texto_direita, sizeof(texto_direita), "%d", contador_bolas);
        ssd1306_draw_string(tela, 105, 4, texto_direita); // Mostra total de bolas
        snprintf(texto_esquerda, sizeof(texto_esquerda), "Desbal");
        ssd1306_draw_string(tela, 0, 5, texto_esquerda);
        snprintf(texto_esquerda, sizeof(texto_esquerda), "Botoes");
        ssd1306_draw_string(tela, 2, 15, texto_esquerda);
        snprintf(texto_esquerda, sizeof(texto_esquerda), "|A|B|");
        ssd1306_draw_string(tela, 0, 25, texto_esquerda);
        desenhar_pinos(tela, &area_render, NUM_LINHAS);

        for (int i = 0; i < NUM_MAX_BOLAS; i++) {
            if (bolas[i].ativa) {
                desenhar_bola(tela, &bolas[i]);
                atualizar_bola(&bolas[i], histograma);
            }
        }

        atualizar_histograma(tela, histograma);
        render_on_display(tela, &area_render);

        if (contador_bolas < NUM_MAX_BOLAS && contador_tempo++ > 5) {
            gerar_bola(bolas);
            contador_tempo = 0;
            contador_bolas++;
        }

        sleep_ms(10);
    }

    return 0;
}
