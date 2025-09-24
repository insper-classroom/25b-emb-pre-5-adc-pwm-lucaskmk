#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define PIN_LED_B 4
#define ADC_CH 2        // GP28 -> ADC2
#define VREF 3.3f
#define ADC_MAX (1 << 12)

static repeating_timer_t timer;
static volatile int current_zone = 0; // usado tanto no main quanto no IRQ

// Callback do timer
bool timer_callback(repeating_timer_t *rt) {
    // Zona 0 → LED sempre apagado
    if (current_zone == 0) {
        gpio_put(PIN_LED_B, 0);
        return true;
    }

    // Alterna LED (toggle)
    bool state = gpio_get(PIN_LED_B);
    gpio_put(PIN_LED_B, !state);

    return true;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, true);

    adc_init();
    adc_gpio_init(28); // GP28
    adc_select_input(ADC_CH);

    int last_zone = -1;

    while (1) {
        uint16_t raw = adc_read();
        float voltage = raw * VREF / ADC_MAX;

        int new_zone;
        if (voltage < 1.0f) {
            new_zone = 0;
        } else if (voltage < 2.0f) {
            new_zone = 1;
        } else {
            new_zone = 2;
        }

        if (new_zone != last_zone) {
            last_zone = new_zone;
            current_zone = new_zone;

            cancel_repeating_timer(&timer);

            if (current_zone == 0) {
                gpio_put(PIN_LED_B, 0); // força apagar
            } else if (current_zone == 1) {
                add_repeating_timer_ms(300, timer_callback, NULL, &timer);
            } else {
                add_repeating_timer_ms(500, timer_callback, NULL, &timer);
            }
        }
    }
}
