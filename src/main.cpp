#include <optional>
#include <stdio.h>
#include "pico/stdlib.h"
#include "button.hpp"
#include "event.hpp"
#include "rotary_encoder.hpp"

#ifndef DEBUG_MODE
#include "bsp/board.h"
#include "hardware/pwm.h"
#include "tusb.h"
#endif

#define ROTARY_0_GPIO_0 0
#define ROTARY_0_GPIO_1 1
#define BUTTON_0_GPIO  16

void pico_led_init() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void pico_set_led(bool on) {
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

void gpio_callback(uint gpio, uint32_t event_mask) {
    record_event(gpio, event_mask);
    // irq is automatically acknowledged
}

int main() {

    #ifndef DEBUG_MODE
    board_init();
    tusb_init();

    gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, false);
    // Get some sensible defaults for the slice configuration. By default, the
    // counter is allowed to wrap over its maximum range (0 to 2**16-1)
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 1.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);
    #endif

    stdio_init_all();
    // pico_led_init();

    sleep_ms(3000);  // LOAD BEARING!!

    printf("Ready!\n");
    
    init_rotary_encoder_handling();
    init_button_handling();

    if (!RotaryEncoder::create_and_register(ROTARY_0_GPIO_0, ROTARY_0_GPIO_1)) {
        panic("Failed to create Rotary Encoder handler!\n");
    }

    if (!Button::create_and_register(BUTTON_0_GPIO)) {
        panic("Failed to create Button handler!\n");
    }

    // pico_set_led(true);

    gpio_set_irq_callback(&gpio_callback);
    enable_rotary_encoder_irq();
    enable_button_irq();
    irq_set_enabled(IO_IRQ_BANK0, true);

    while (true) {

        std::optional<Event> maybe_event = pop_event();
        while (maybe_event.has_value()) {
            handle_rotary_encoder_event(maybe_event.value());
            handle_button_event(maybe_event.value());
            maybe_event = pop_event();
        }
        #ifndef DEBUG_MODE
        tud_task(); // tinyusb task
        #endif
    }

    return 0;
}

#ifndef DEBUG_MODE
//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    return;
    /*
    if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT && buffer[0] == 2 && bufsize >= sizeof(light_data)) //light data
    {
        size_t i = 0;
        for (i; i < sizeof(light_data); i++)
        {
            light_data.raw[i] = buffer[i + 1];
        }
    }
    */

    // echo back anything we received from host
    tud_hid_report(0, buffer, bufsize);
}
#endif
