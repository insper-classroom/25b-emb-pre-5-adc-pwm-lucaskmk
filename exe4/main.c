#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define LED_PIN 4
#define ADC_PIN 28
#define ADC_CH  2
#define VREF    3.3f
#define ADC_MAX (1 << 12)

static repeating_timer_t timer;

bool led_blink_callback(repeating_timer_t *t) {
    bool state = gpio_get(LED_PIN);
    gpio_put(LED_PIN, !state);
    return true;
}

int main() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CH);

    int current_zone = -1;

    while (true) {
        uint16_t raw = adc_read();
        float voltage = raw * VREF / ADC_MAX;

        int zone;
        if (voltage <= 1.0f) {
            zone = 0;
        } else if (voltage <= 2.0f) {
            zone = 1;
        } else {
            zone = 2;
        }

        if (zone != current_zone) {
            current_zone = zone;
            cancel_repeating_timer(&timer);

            if (zone == 0) {
                gpio_put(LED_PIN, 0);
            } else if (zone == 1) {
                add_repeating_timer_ms(300, led_blink_callback, NULL, &timer);
            } else {
                add_repeating_timer_ms(500, led_blink_callback, NULL, &timer);
            }
        }
    }
}
