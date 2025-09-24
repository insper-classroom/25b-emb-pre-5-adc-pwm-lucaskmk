#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

#define PIN_LED_B 4
#define ADC_CH 2
#define VREF 3.3f
#define ADC_MAX (1 << 12)

typedef struct {
    int zone;
    bool state;
} led_control_t;

static repeating_timer_t timer;

bool timer_callback(repeating_timer_t *rt) {
    led_control_t *ctrl = (led_control_t *)rt->user_data;
    if (ctrl->zone == 0) {
        gpio_put(PIN_LED_B, 0);
        ctrl->state = false;
        return true;
    }
    ctrl->state = !ctrl->state;
    gpio_put(PIN_LED_B, ctrl->state);
    return true;
}

int main() {
    stdio_init_all();
    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, true);
    adc_init();
    adc_gpio_init(28);
    adc_select_input(ADC_CH);

    led_control_t ctrl = {0, false};
    int last_zone = -1;

    while (1) {
        uint16_t raw = adc_read();
        float voltage = raw * VREF / ADC_MAX;

        int new_zone;
        if (voltage < 1.0f) new_zone = 0;
        else if (voltage < 2.0f) new_zone = 1;
        else new_zone = 2;

        if (new_zone != last_zone) {
            last_zone = new_zone;
            ctrl.zone = new_zone;
            cancel_repeating_timer(&timer);
            if (ctrl.zone == 0) {
                gpio_put(PIN_LED_B, 0);
                ctrl.state = false;
            } else if (ctrl.zone == 1) {
                add_repeating_timer_ms(300, timer_callback, &ctrl, &timer);
            } else {
                add_repeating_timer_ms(500, timer_callback, &ctrl, &timer);
            }
        }
    }
}
