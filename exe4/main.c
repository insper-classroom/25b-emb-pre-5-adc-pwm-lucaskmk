#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

#define LED_BLUE 4
#define ADC_GPIO 28
#define ADC_CH   2
#define VREF     3.3f
#define ADC_MAX  4096.0f

static repeating_timer_t led_timer;
static bool blink_cb(repeating_timer_t *rt) {
    gpio_put(LED_BLUE, !gpio_get(LED_BLUE));
    return true;
}

int main() {
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 0);

    adc_init();
    adc_gpio_init(ADC_GPIO);
    adc_select_input(ADC_CH);

    int zone_atual = -1;

    while (true) {
        uint16_t raw = adc_read();
        float v = (raw * VREF) / ADC_MAX;
        int nova_zona;
        if (v <= 1.0f) {
            nova_zona = 0;
        } else if (v <= 2.0f) {
            nova_zona = 1;
        } else {
            nova_zona = 2;
        }
        if (nova_zona != zone_atual) {
            zone_atual = nova_zona;
            cancel_repeating_timer(&led_timer);
            if (zone_atual == 0) {
                gpio_put(LED_BLUE, 0);
            } else if (zone_atual == 1) {
                add_repeating_timer_ms(300, blink_cb, NULL, &led_timer);
            } else {
                add_repeating_timer_ms(500, blink_cb, NULL, &led_timer);
            }
        }
    }
}
