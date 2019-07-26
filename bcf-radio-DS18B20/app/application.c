#include <application.h>
#include <bc_ds18b20.h>

#define TEMPERATURE_DS18B20_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define TEMPERATURE_DS18B20_PUB_VALUE_CHANGE 0.4f
#define UPDATE_SERVICE_INTERVAL (5 * 1000)

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

//DS18B20 instance
static bc_ds18b20_t ds18d20;

void handler_ds18b20(bc_ds18b20_t *s, bc_ds18b20_event_t e, void *p);

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
    }

    // Logging in action
    bc_log_info("Button event handler - event: %i", event);
}

void application_init(void)
{
    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    //Initialize DS18B20
    bc_ds18b20_init(&ds18d20, BC_DS18B20_RESOLUTION_BITS_12);
    bc_ds18b20_set_event_handler(&ds18d20, handler_ds18b20, NULL);
    bc_ds18b20_set_update_interval(&ds18d20, UPDATE_SERVICE_INTERVAL);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);
}
/*
void application_task(void)
{
    // Logging in action
    bc_log_debug("application_task run");

    // Plan next run this function after 1000 ms
    bc_scheduler_plan_current_from_now(1000);
}*/

void handler_ds18b20(bc_ds18b20_t *s, bc_ds18b20_event_t e, void *p)
{
    (void) p;

    float value = NAN;

    if (e == bc_ds18b20_EVENT_UPDATE)
    {
        bc_ds18b20_get_temperature_celsius(s, &value);
        bc_log_info("DS18B20  - value: %f", value);
        
    }
    bc_log_info("Button event handler - event: %i", e);
}