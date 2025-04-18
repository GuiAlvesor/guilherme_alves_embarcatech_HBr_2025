# Sistema de Contagem com Display OLED

## Contexto Geral do Projeto

Este projeto implementa um sistema de contagem regressiva com interface visual utilizando um display OLED e botões de controle. O sistema é baseado no microcontrolador Raspberry Pi Pico e utiliza comunicação I2C para controlar o display OLED SSD1306.

O sistema permite:
- Contagem regressiva de 9 a 0
- Contagem de cliques em um botão específico
- Reset do contador através de um botão
- Visualização em tempo real através de um display OLED

### Aplicações Práticas
- Sistema de contagem regressiva para jogos
- Temporizador simples
- Contador de eventos
- Interface de teste para desenvolvimento

## Arquitetura do Sistema

O sistema é composto por três camadas principais:

1. **Camada de Hardware**:
   - Raspberry Pi Pico (microcontrolador)
   - Display OLED SSD1306 (interface visual)
   - Botões de controle (A e B)

2. **Camada de Drivers**:
   - Driver SSD1306 para comunicação I2C
   - Sistema de interrupções para botões
   - Timer para contagem regressiva

3. **Camada de Aplicação**:
   - Lógica de contagem regressiva
   - Sistema de contagem de cliques
   - Interface de usuário no display

### Diagrama de Conexões
```
Raspberry Pi Pico
├── I2C1 (SDA: GPIO 14, SCL: GPIO 15)
│   └── Display OLED SSD1306
├── GPIO 5
│   └── Botão A (Reset)
└── GPIO 6
    └── Botão B (Contagem)
```

## Componentes Utilizados

### 1. Raspberry Pi Pico
- Microcontrolador baseado no chip RP2040
- 2 núcleos ARM Cortex-M0+ @ 133MHz
- 264KB de RAM
- 2MB de memória flash
- 30 GPIOs multifuncionais
- 2x SPI, 2x I2C, 2x UART
- 16 canais PWM
- [Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)

### 2. Display OLED SSD1306
- Display monocromático de 128x64 pixels
- Interface I2C (endereço padrão: 0x3C)
- Tensão de operação: 3.3V
- Taxa de atualização: 100Hz
- Consumo de energia: ~20mA
- [Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

### 3. Botões
- Botão A (GPIO 5): Reset do contador
- Botão B (GPIO 6): Contagem de cliques
- Resistores de pull-up internos (50kΩ)
- Configuração de interrupção por borda
- Tempo de debounce: 500ms

## Fluxograma de Lógica do Programa

```
┌─────────────────┐
│  Inicialização  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Configura I2C  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Configura Display│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Configura Botões│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Inicia Timer   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│    Main Loop    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Tick do Timer?  │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
    ▼         ▼
┌─────────┐  ┌─────────┐
│Decrementa│  │Botão A  │
│Contador  │  │Pressionado?│
└────┬────┘  └────┬────┘
     │            │
     │      ┌─────┴─────┐
     │      │           │
     │      ▼           ▼
     │  ┌─────────┐  ┌─────────┐
     │  │Reset    │  │Botão B  │
     │  │Contador │  │Pressionado?│
     │  └────┬────┘  └────┬────┘
     │       │            │
     │       │      ┌─────┴─────┐
     │       │      │           │
     │       │      ▼           │
     │       │  ┌─────────┐     │
     │       │  │Incrementa│     │
     │       │  │Cliques   │     │
     │       │  └────┬────┘     │
     │       │       │          │
     │       │       │          │
     ▼       ▼       ▼          │
┌─────────────────┐  │          │
│ Atualiza Display│  │          │
└────────┬────────┘  │          │
         │           │          │
         ▼           │          │
┌─────────────────┐  │          │
│   Main Loop     │◄─┘          │
└─────────────────┘◄────────────┘
```

## Análise do Código Principal

### 1. Estruturas e Variáveis Globais
```c
// Variáveis de estado
volatile int contador = 9;        // Contador regressivo (9 a 0)
volatile int cliques = 0;         // Contador de cliques
volatile bool tick = false;       // Flag de tick do timer
volatile bool foi_resetado = false; // Flag de reset
volatile bool atualiza_display = false; // Flag de atualização do display

// Configuração do display
struct render_area display_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1,
};
```

### 2. Funções Principais

#### `on_timer(struct repeating_timer *t)`
- Função de callback do timer
- Define a flag `tick` como true a cada 1 segundo
- Retorna true para manter o timer ativo
- Frequência: 1Hz (1000ms)

#### `gpio_callback(uint gpio, uint32_t events)`
- Manipulador de interrupções dos botões
- Botão A (GPIO 5):
  - Reseta o contador para 9
  - Zera os cliques se o contador estiver em 0
  - Interrupção por borda de subida
- Botão B (GPIO 6):
  - Incrementa o contador de cliques
  - Implementa debounce de 500ms
  - Só funciona se o contador não estiver em 0
  - Interrupção por borda de descida

#### `atualizar_display()`
- Limpa o buffer do display
- Formata e exibe o contador regressivo
- Formata e exibe o número de cliques
- Renderiza o conteúdo no display
- Otimização: Atualiza apenas quando necessário

#### `main()`
1. Inicialização:
   - Configura I2C (400kHz)
   - Configura GPIOs para I2C
   - Inicializa display OLED
   - Configura botões com interrupções
   - Inicia timer de 1 segundo

2. Loop Principal:
   - Verifica tick do timer
   - Decrementa contador se necessário
   - Atualiza display quando solicitado
   - Mantém o sistema responsivo

## Considerações Técnicas

1. **Debounce de Botões**:
   - Implementado para o botão B
   - Tempo de debounce: 500ms
   - Evita múltiplos registros de um único clique
   - Uso de timestamp para controle de tempo

2. **Sincronização**:
   - Uso de flags volatile para comunicação entre interrupções e loop principal
   - Timer de 1 segundo para contagem regressiva
   - Atualização do display apenas quando necessário
   - Proteção contra race conditions

3. **Otimizações**:
   - Uso de tight_loop_contents() para economia de energia
   - Atualização do display apenas quando há mudanças
   - Buffer local para renderização do display
   - Minimização de operações I2C

4. **Segurança**:
   - Proteção contra overflow no contador de cliques
   - Verificação de estados inválidos
   - Tratamento de erros de comunicação I2C

## Dependências e Arquivos do Projeto

### 1. Arquivos Principais

#### `main.c`
- **Função**: Arquivo principal do programa
- **Conteúdo**:
  - Inicialização do sistema
  - Configuração de hardware
  - Loop principal
  - Manipulação de interrupções
  - Lógica de contagem e display
- **Dependências**:
  - pico/stdlib.h
  - hardware/i2c.h
  - hardware/timer.h
  - hardware/gpio.h
  - pico/time.h

#### `CMakeLists.txt`
- **Função**: Arquivo de configuração do sistema de build
- **Conteúdo**:
  - Configuração do projeto
  - Definição de dependências
  - Configuração de flags de compilação
  - Definição de alvos de build

### 2. Arquivos do Driver SSD1306

#### `ssd1306.h`
- **Função**: Cabeçalho principal do driver SSD1306
- **Conteúdo**:
  - Definições de constantes
  - Protótipos de funções
  - Estruturas de dados
  - Configurações do display

#### `ssd1306_font.h`
- **Função**: Definição de fontes para o display
- **Conteúdo**:
  - Tabela de caracteres
  - Definições de tamanho de fonte
  - Funções de renderização de texto
- **Caracteres Disponíveis**:
  - Letras Maiúsculas: A-Z
  - Letras Minúsculas: a-z
  - Números: 0-9
  - Símbolos: : (dois pontos)
- **Formato dos Caracteres**:
  - Cada caractere é definido por 8 bytes (8x8 pixels)
  - Matriz de 8x8 bits para cada caractere
  - Caracteres são renderizados em modo monocromático
- **Exemplo de Caractere**:
  ```
  Caractere 'A':
  0x78, 0x14, 0x12, 0x11, 0x12, 0x14, 0x78, 0x00
  ```

#### `ssd1306_i2c.h`
- **Função**: Interface I2C para o display
- **Conteúdo**:
  - Protótipos de funções I2C
  - Definições de pinos
  - Configurações de comunicação

#### `ssd1306_i2c.c`
- **Função**: Implementação da comunicação I2C
- **Conteúdo**:
  - Funções de inicialização
  - Funções de escrita/leitura
  - Manipulação de buffer
  - Comandos do display

### 3. Dependências do Sistema

#### pico-sdk
- **Função**: SDK oficial do Raspberry Pi Pico
- **Componentes utilizados**:
  - `pico/stdlib.h`: Funções básicas do sistema
  - `pico/time.h`: Manipulação de tempo
- **Versão recomendada**: 1.5.1 ou superior

#### hardware/i2c
- **Função**: Driver de comunicação I2C
- **Características**:
  - Suporte a múltiplos dispositivos
  - Configuração de velocidade
  - Manipulação de interrupções

#### hardware/timer
- **Função**: Gerenciamento de temporizadores
- **Características**:
  - Timer de alta precisão
  - Suporte a callbacks
  - Configuração de períodos

#### hardware/gpio
- **Função**: Gerenciamento de GPIOs
- **Características**:
  - Configuração de pinos
  - Manipulação de interrupções
  - Suporte a pull-up/pull-down

### 4. Arquivos de Configuração

#### `wokwi-project.txt`
- **Função**: Configuração do ambiente de simulação Wokwi
- **Conteúdo**:
  - Definição de componentes
  - Configurações de simulação
  - Conexões virtuais

#### `diagram.json`
- **Função**: Diagrama do circuito
- **Conteúdo**:
  - Layout dos componentes
  - Conexões elétricas
  - Configurações visuais

### 5. Estrutura de Diretórios
```
projeto/
├── main.c
├── CMakeLists.txt
├── ssd1306/
│   ├── ssd1306.h
│   ├── ssd1306_font.h
│   ├── ssd1306_i2c.h
│   └── ssd1306_i2c.c
├── wokwi-project.txt
└── diagram.json
```

## Configuração do Ambiente

1. **Requisitos**:
   - Raspberry Pi Pico
   - Display OLED SSD1306
   - 2 botões
   - Jumpers e protoboard
   - Fonte de alimentação 3.3V

2. **Conexões**:
   ```
   Pico Pin    -> Componente
   ---------    -----------
   GPIO 14     -> SDA (Display)
   GPIO 15     -> SCL (Display)
   GPIO 5      -> Botão A
   GPIO 6      -> Botão B
   3.3V        -> VCC (Display)
   GND         -> GND (Display)
   ```

3. **Compilação**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Troubleshooting

1. **Display não liga**:
   - Verificar conexões I2C
   - Confirmar alimentação 3.3V
   - Verificar endereço I2C (0x3C)

2. **Botões não respondem**:
   - Verificar conexões GPIO
   - Confirmar pull-up resistors
   - Verificar interrupções configuradas

3. **Contador não atualiza**:
   - Verificar timer configurado
   - Confirmar flags volatile
   - Verificar loop principal

## Hardware BitDogLab

Este projeto é parte do curso EmbarcaTech e utiliza a placa BitDogLab, uma plataforma educacional STEAM que incorpora o Raspberry Pi Pico.

### Características da Placa BitDogLab
- Microcontrolador: Raspberry Pi Pico (RP2040)
- Display OLED SSD1306 128x64 integrado
- Botões de usuário:
  - BTN_A (GPIO 5): Botão multifunção
  - BTN_B (GPIO 6): Botão multifunção
- Conexões I2C dedicadas:
  - SDA: GPIO 14
  - SCL: GPIO 15
- Alimentação: USB ou fonte externa 3.3V
- Proteções de hardware integradas
- Design otimizado para projetos educacionais

### Recursos Utilizados neste Projeto
- Display OLED integrado via I2C
  - Resolução: 128x64 pixels
  - Interface: I2C (endereço 0x3C)
  - Conexão direta sem necessidade de componentes externos
- Botões de usuário
  - Pull-up interno habilitado
  - Debounce por software
  - Interrupções configuradas
- Comunicação I2C
  - Velocidade: 400kHz
  - Pinos dedicados com pull-up integrado

### Vantagens do Hardware
- Integração completa dos componentes
- Proteção contra conexões incorretas
- Design robusto para ambiente educacional
- Compatibilidade com exemplos e bibliotecas do curso EmbarcaTech
- Facilidade de programação via USB

### Links Relacionados
- [Repositório BitDogLab](https://github.com/BitDogLab)
- [BitDogLab-C: Exemplos em C](https://github.com/BitDogLab/BitDogLab-C)
- [Documentação do Curso EmbarcaTech](https://github.com/BitDogLab/EmbarcaTech-2-Alunos)

