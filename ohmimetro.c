#include <stdio.h>
#include "pico/stdlib.h"
#include <math.h>
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "pico/bootrom.h"
#include "lib/neopixel.h"

#define ADC_PIN 28 // GPIO para o voltímetro

// Definições de botões
#define BUTTON_A 5
#define BUTTON_B 6
#define BUTTON_JOYSTICK 22

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

char str_resistencia[6]; // Buffer para armazenar a string
char str_resistencia_comercial[6];
char str_adc[6]; // Buffer para armazenar a string

// Série E24 normalizada de 1.0 a 9.1
const float E24[] = {
    1.0f, 1.1f, 1.2f, 1.3f, 1.5f, 1.6f, 1.8f, 2.0f,
    2.2f, 2.4f, 2.7f, 3.0f, 3.3f, 3.6f, 3.9f, 4.3f,
    4.7f, 5.1f, 5.6f, 6.2f, 6.8f, 7.5f, 8.2f, 9.1f
};
const int E24_SIZE = 24;

typedef struct {
    const char *nome;
    int tamanho;
    int r, g, b;
} Cor;

Cor cores[] = {
    {"Preto",    5, 0,   0,   0},  // 0
    {"Marrom",   6, 18,  6,   0},  // 1
    {"Vermelho", 8, 25,  0,   0},  // 2
    {"Laranja",  7, 12,  1,   0},  // 3
    {"Amarelo",  7, 25,  25,  0},  // 4
    {"Verde",    5, 0,   25,  0},  // 5
    {"Azul",     4, 0,   0,   25}, // 6
    {"Violeta",  7, 12,  0,   12}, // 7
    {"Cinza",    5, 1,   1,   1},  // 8
    {"Branco",   6, 25,  25,  25}  // 9
};

ssd1306_t ssd; //Estrutura do display

//Prototipos de funções
void setup();
void display_init();
static void gpio_button_joystick_handler(uint gpio, uint32_t events);

void atualiza_display(float resistencia, float resistencia_e24, float adc);
float ler_adc();
float encontraProximoE24(float resistencia);
void mostra_faixa_matriz();

uint8_t faixa1, faixa2, multiplicador;

int main()
{
    setup();
    adc_select_input(2); // Seleciona o canal 2
    float adc = 0;
    float resistencia = 500;
    float resistencia_e24;

    while (true) {
        //adc = ler_adc();
        //resistencia = (R_conhecido * adc) / (ADC_RESOLUTION - adc);

        
        resistencia_e24 = encontraProximoE24(resistencia);
        atualiza_display(resistencia, resistencia_e24, adc);
        mostra_faixa_matriz();
        resistencia = (resistencia + 100);
        if(resistencia > 20000) resistencia = 500;
        sleep_ms(1000);
    }
}


float ler_adc(){

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

    return media;
}

float encontraProximoE24(float resistencia) {
    int expoenteDez = 0;
    float resNorm = resistencia;

    // traz resNorm para [1.0, 10.0)
    while (resNorm >= 10.0f) { 
        resNorm /= 10.0f; 
        expoenteDez++; 
    }
    while (resNorm < 1.0f) { 
        resNorm *= 10.0f; 
        expoenteDez--; 
    }

    // procura o índice do valor E24 mais próximo
    int indiceMelhor = 0;
    float menorDif = fabsf(resNorm - E24[0]);
    for (int i = 1; i < E24_SIZE; i++) {
        float dif = fabsf(resNorm - E24[i]);
        if (dif < menorDif) {
            menorDif     = dif;
            indiceMelhor = i;
        }
    }

    //Ex R = 9.9. A função acima faria com que o indice melhor fosse o 9.1 sendo que 10.0 seria o mais próximo). Esse if concerta isso
    if(fabsf(resNorm - 10.0) < menorDif){
        indiceMelhor = 0;
        menorDif = 1.0;
        expoenteDez++;
    }

    // calcula as duas primeiras faixas (dois dígitos) e o multiplicador
    int doisDigitos = (int)(E24[indiceMelhor] * 10.0f + 0.5f); // ex: 2.2 → 22
    faixa1        = doisDigitos / 10;
    faixa2        = doisDigitos % 10;
    multiplicador = expoenteDez - 1;

    // reconstrói e retorna o valor comercial completo
    return E24[indiceMelhor] * powf(10.0f, expoenteDez);
}

void mostra_faixa_matriz(){

    uint8_t r = cores[faixa1].r;
    uint8_t g = cores[faixa1].g;
    uint8_t b = cores[faixa1].b;

    npSetLED(npGetIndex(1,1), r, g, b);
    npSetLED(npGetIndex(1,2), r, g, b);
    npSetLED(npGetIndex(1,3), r, g, b);

    r = cores[faixa2].r;
    g = cores[faixa2].g;
    b = cores[faixa2].b;

    npSetLED(npGetIndex(2,1), r, g, b);
    npSetLED(npGetIndex(2,2), r, g, b);
    npSetLED(npGetIndex(2,3), r, g, b);

    r = cores[multiplicador].r;
    g = cores[multiplicador].g;
    b = cores[multiplicador].b;
    
    npSetLED(npGetIndex(3,1), r, g, b);
    npSetLED(npGetIndex(3,2), r, g, b);
    npSetLED(npGetIndex(3,3), r, g, b);

    npWrite();
}

void atualiza_display(float resistencia, float resistencia_e24, float adc){

    sprintf(str_resistencia, "%1.0f", resistencia); // Converte o float em string 
    sprintf(str_resistencia_comercial, "%1.0f", resistencia_e24); // Converte o float em string 
    sprintf(str_adc, "%1.0f", adc);   // Converte o inteiro em string
    
    ssd1306_fill(&ssd, false);                          // Limpa o display

    ssd1306_rect(&ssd, 1, 1, 123, 63, true, false);      // Desenha um retângulo

    ssd1306_draw_string(&ssd, "ADC", 13, 4);          // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.", 50, 4);    // Desenha uma string
    ssd1306_draw_string(&ssd, str_adc, 8, 13);           // Desenha uma string
    ssd1306_draw_string(&ssd, str_resistencia, 59, 13);          // Desenha uma string
    ssd1306_line(&ssd, 44, 1, 44, 22, true);           // Desenha uma linha vertical
    ssd1306_line(&ssd, 3, 22, 123, 22, true);           // Desenha uma linha horizontal

    ssd1306_draw_string(&ssd, "R.Comer: ", 3, 24);          // Desenha uma string
    ssd1306_draw_string(&ssd, str_resistencia_comercial, 69, 24);          // Desenha uma string
    ssd1306_line(&ssd, 3, 33, 123, 33, true);           // Desenha uma linha horizontal

    ssd1306_draw_string(&ssd, "1-", 3 , 36);
    ssd1306_draw_string(&ssd, cores[faixa1].nome, (128 - cores[faixa1].tamanho * 8)/2, 36);  //Centraliza as strings

    ssd1306_draw_string(&ssd, "2-", 3 , 45);
    ssd1306_draw_string(&ssd, cores[faixa2].nome, (128 - cores[faixa2].tamanho * 8)/2, 45); 

    ssd1306_draw_string(&ssd, "M.", 3 , 54);
    ssd1306_draw_string(&ssd, cores[multiplicador].nome, (128 - cores[multiplicador].tamanho * 8)/2, 54); 

    

    ssd1306_send_data(&ssd);   
}

void setup(){

    stdio_init_all();

    //Inicia matriz de leds
    npInit(LED_PIN);

    //Inicializa display
    display_init();

    adc_init();
    
    adc_gpio_init(28);  

    //Configuração dos Botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_button_joystick_handler);

    
}

void display_init(){
  // Configuração I2C a 400kHz
  i2c_init(I2C_PORT, 400 * 1000);
    
  // Configura pinos I2C
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA); // Ativa pull-ups internos
  gpio_pull_up(I2C_SCL);



  // Inicialização do controlador SSD1306
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
  ssd1306_config(&ssd);
  ssd1306_send_data(&ssd);

  // Limpa a tela inicialmente
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);
}

static void gpio_button_joystick_handler(uint gpio, uint32_t events) {

    printf("\nHABILITANDO O MODO GRAVAÇÃO\n");

    // Adicionar feedback no display OLED
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "  HABILITANDO", 5, 25);
    ssd1306_draw_string(&ssd, " MODO GRAVACAO", 5, 38);
    ssd1306_send_data(&ssd);

    reset_usb_boot(0,0); // Reinicia
}