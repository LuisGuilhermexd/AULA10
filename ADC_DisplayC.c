#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições dos pinos
#define LED_RED_PIN 12
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 13
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_BUTTON_PIN 22
#define BUTTON_A_PIN 5
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define I2C_PORT i2c1
#define DISPLAY_ADDRESS 0x3C

// Variáveis globais
volatile bool green_led_state = false; // Estado do LED verde
volatile bool pwm_enabled = true;     // Estado do PWM
volatile uint8_t border_style = 0;    // Estilo da borda do display

// Função de debouncing
bool debounce(uint gpio) {
    sleep_ms(20);
    return gpio_get(gpio);
}

// Interrupção para o botão do joystick
void joystick_button_irq(uint gpio, uint32_t events) {
    if (debounce(gpio)) {
        green_led_state = !green_led_state; // Alterna o estado do LED verde
        border_style = (border_style + 1) % 3; // Altera o estilo da borda
    }
}

// Interrupção para o botão A
void button_a_irq(uint gpio, uint32_t events) {
    if (debounce(gpio)) {
        pwm_enabled = !pwm_enabled;
        if (!pwm_enabled) {
            pwm_set_gpio_level(LED_RED_PIN, 0);    // Desativa LED Vermelho
            pwm_set_gpio_level(LED_GREEN_PIN, 0);  // Desativa LED Verde
            pwm_set_gpio_level(LED_BLUE_PIN, 0);   // Desativa LED Azul
        }
    }
}

// Configuração do PWM para os LEDs
void setup_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, 4095);
    pwm_set_enabled(slice_num, true);
}

// Atualiza a borda do display
void update_display_border(ssd1306_t *display) {
    ssd1306_fill(display, false);
    switch (border_style) {
        case 0:
            ssd1306_rect(display, 0, 0, 128, 64, true, false);
            break;
        case 1:
            ssd1306_rect(display, 2, 2, 124, 60, true, false);
            break;
        case 2:
            ssd1306_rect(display, 4, 4, 120, 56, true, false);
            break;
    }
    ssd1306_send_data(display);
}

int main() {
    // Inicialização do SDK
    stdio_init_all();
    adc_init();

    // Configuração dos ADCs para o joystick
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    adc_select_input(0); // Canal 0 (GPIO 26 - Eixo X)

    // Configuração dos LEDs PWM
    setup_pwm(LED_RED_PIN);
    setup_pwm(LED_GREEN_PIN);
    setup_pwm(LED_BLUE_PIN);

    // Configuração dos botões com interrupções
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &joystick_button_irq);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_irq);

    // Configuração do I2C para o display SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicialização do display SSD1306
    ssd1306_t display;
    ssd1306_init(&display, 128, 64, false, DISPLAY_ADDRESS, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Loop principal
    while (true) {
        if (pwm_enabled) {
            // Leitura do eixo X (GPIO 26)
            adc_select_input(0);
            uint16_t x_value = adc_read();

            // Leitura do eixo Y (GPIO 27)
            adc_select_input(1);
            uint16_t y_value = adc_read();

            // Zona morta para evitar que os LEDs acendam quando o joystick estiver em repouso
            if (x_value < 1800 || x_value > 2200) {
                if (x_value < 2048) {
                    pwm_set_gpio_level(LED_RED_PIN, (2048 - x_value) * 2);
                } else {
                    pwm_set_gpio_level(LED_RED_PIN, (x_value - 2048) * 2);
                }
            } else {
                pwm_set_gpio_level(LED_RED_PIN, 0);
            }

            if (y_value < 1800 || y_value > 2200) {
                if (y_value < 2048) {
                    pwm_set_gpio_level(LED_BLUE_PIN, (2048 - y_value) * 2);
                } else {
                    pwm_set_gpio_level(LED_BLUE_PIN, (y_value - 2048) * 2);
                }
            } else {
                pwm_set_gpio_level(LED_BLUE_PIN, 0);
            }
        }

        // Controle do LED Verde
        pwm_set_gpio_level(LED_GREEN_PIN, green_led_state ? 4095 : 0);

        // Atualização da posição do quadrado no display
        adc_select_input(0);
        uint16_t x_value = adc_read();
        adc_select_input(1);
        uint16_t y_value = adc_read();

        // Centraliza o quadrado com base no valor do joystick
        uint16_t x_pos = 60 + ((x_value - 2048) * 60) / 2048; // 60 = (128 - 8) / 2
        uint16_t y_pos = 28 + ((y_value - 2048) * 28) / 2048; // 28 = (64 - 8) / 2

        // Limita os valores para evitar que o quadrado saia da tela
        x_pos = (x_pos < 0) ? 0 : (x_pos > 120) ? 120 : x_pos;
        y_pos = (y_pos < 0) ? 0 : (y_pos > 56) ? 56 : y_pos;

        // Atualiza o display
        ssd1306_fill(&display, false);
        update_display_border(&display);
        ssd1306_rect(&display, x_pos, y_pos, 8, 8, true, true);
        ssd1306_send_data(&display);

        sleep_ms(10);
    }
}