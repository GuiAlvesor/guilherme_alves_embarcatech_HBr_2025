# Sistema de Resfriamento 

Esse projeto implementa o monitoramento constante da variação de temperatura e controle via pwm de motores para o controle dessa.

## Objetivo

Monitorar de forma precisa e controlar a temperatura e sua variação usando:


- Display OLED para exibir informações em tempo real
- Botões para incrementar ou decrementar a temperatura limite
- Fan para resfriar o ambiente 
- Buzzers para alertar temperaturas acima do tolerável


##  Lista de materiais: 

| Componente                    | Conexão na BitDogLab      |
|-------------------------------|---------------------------|
| BitDogLab (RP2040)            | -                         |
| Sensor Onboard Temperatura    | -                         |
| Buzzer 1                      | GPIO 2                    |
| Buzzer 2                      | GPIO 3                    |
| Display OLED I2C              | SDA: GPIO14 / SCL: GPIO15 |
| Botão A                       | GPIO 5                    |
| Botão B                       | GPIO 6                    |
| Fan                           | GPIO 8 E 9                |


## Execução

1. Abra o projeto no VS Code, usando o ambiente com suporte ao SDK do Raspberry Pi Pico (CMake + compilador ARM);
2. Compile o projeto normalmente (Ctrl+Shift+B no VS Code ou via terminal com cmake e make);
3. Conecte sua BitDogLab via cabo USB e coloque a Pico no modo de boot (pressione o botão BOOTSEL e conecte o cabo);
4. Copie o arquivo .uf2 gerado para a unidade de armazenamento que aparece (RPI-RP2);
5. A Pico reiniciará automaticamente e começará a executar o código;
6. O histograma será atualizado no display OLED e os eventos simulados aparecerão na matriz de LEDs.

## Lógica

- O sensor realiza medição de temperatura;
- O display OLED representam o valor atual da temperatura e o limite estabelicido;
  Os botões incrementam ou decrementam o valor limite;
- O valor da temperatura será comparado ao contador (limite);
- O fan inicia a ventilação caso o valor da temperatura exceda o valor máximo;

##  Arquivos

- `sistema_resfriamento.c`: Código principal do projeto;

## 🖼️ Imagens do Projeto

### Matriz de LEDs durante execução
![sistema_resfriamento](./assets/images/sistema_resfriamento.jpg)


## 📜 Licença
MIT License - MIT GPL-3.0.
