# Bit Ohm - Ohmímetro na BitDogLab

Um sistema de medição de resistores utilizando a plataforma BitDogLab, capaz de calcular o valor aproximado da resistência, identificar o valor comercial E24 mais próximo, e exibir as faixas de cores correspondentes.
## Vídeo Demonstrativo
[![Assista ao vídeo no YouTube](https://img.youtube.com/vi/HY4chSQSjx0/hqdefault.jpg)](https://youtu.be/HY4chSQSjx0)

## Funcionalidades Principais

- Leitura da resistência do resistor conectado utilizando o ADC do RP2040.
- Cálculo da resistência real com base no divisor de tensão.
- Ajuste automático para o valor comercial mais próximo da série E24 (5% de tolerância).
- Exibição das informações no Display OLED:
  - Valor calculado da resistência.
  - Valor comercial E24 correspondente.
  - Cores das três primeiras faixas do resistor (faixa 1, faixa 2 e multiplicador).
- Representação visual das faixas de cores na Matriz de LEDs RGB.
- **Botão Joystick:** Utilizado para entrar em Modo BOOTSEL (bootloader).


## Componentes Utilizados

| Componente              | GPIO/Pinos         | Função                                              |
|--------------------------|--------------------|-----------------------------------------------------|
| ADC                      | 28                 | Leitura da tensão para cálculo da resistência       |
| Display OLED SSD1306     | 14 (SDA) e 15 (SCL) | Exibição dos valores calculados e cores             |
| Matriz de LEDs RGB       | 7                  | Exibição visual das cores correspondentes           |
| Botão do Joystick        | 22                  | Entrar em modo BOOTSEL para reprogramação            |


## ⚙️ Instalação e Uso

1. **Pré-requisitos**
   - Clonar o repositório:
     ```bash
     git clone https://github.com/JotaPablo/ohmimetro.git
     cd ohmimetro
     ```
   - Instalar o **Visual Studio Code** com as seguintes extensões:
     - **Raspberry Pi Pico SDK**
     - **Compilador ARM GCC**

2. **Compilação**
   - Compile o projeto no terminal:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```
   - Ou utilize a extensão da Raspberry Pi Pico no VS Code.

3. **Execução**
   - **Na placa física:** 
     - Conecte a placa ao computador em modo **BOOTSEL**.
     - Copie o arquivo `.uf2` gerado na pasta `build` para o dispositivo identificado como `RPI-RP2`, ou envie através da extensão da Raspberry Pi Pico no VS Code.