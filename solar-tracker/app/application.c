#include <application.h>
#include <bc_servo.h>

uint16_t ldr1;
uint16_t ldr2;
uint16_t ldr3;
uint16_t ldr4;
uint8_t ver = 100;
uint8_t hor = 60;

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

bc_servo_t horizontal;
bc_servo_t vertical;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
    }

    // Logging in action
    bc_log_info("Button event handler - event: %i", event);
}

void adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *event_param)
{
    if (event == BC_ADC_EVENT_DONE)
    {
        uint16_t result;

        if (bc_adc_async_get_value(channel, &result))
        {
            //bc_log_debug("ADC: Channel = %05u", channel);
            //bc_log_debug("ADC: Value = %05u", result);

            if (channel == 5){
                ldr1 = result;
            }
            if (channel == 4){
                ldr2 = result;
            }
            if (channel == 2){
                ldr3 = result;
            }
            if (channel == 0){
                ldr4 = result;
            }
        }
    }
}

void application_init(void)
{
    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);
    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_adc_init();
    bc_adc_set_event_handler(BC_ADC_CHANNEL_A5, adc_event_handler, NULL);
    bc_adc_oversampling_set(BC_ADC_CHANNEL_A5, BC_ADC_OVERSAMPLING_32);
    bc_adc_resolution_set(BC_ADC_CHANNEL_A5, BC_ADC_RESOLUTION_12_BIT);

    bc_adc_set_event_handler(BC_ADC_CHANNEL_A4, adc_event_handler, NULL);
    bc_adc_oversampling_set(BC_ADC_CHANNEL_A4, BC_ADC_OVERSAMPLING_32);
    bc_adc_resolution_set(BC_ADC_CHANNEL_A4, BC_ADC_RESOLUTION_12_BIT);  

    bc_adc_set_event_handler(BC_ADC_CHANNEL_A2, adc_event_handler, NULL);
    bc_adc_oversampling_set(BC_ADC_CHANNEL_A2, BC_ADC_OVERSAMPLING_32);
    bc_adc_resolution_set(BC_ADC_CHANNEL_A2, BC_ADC_RESOLUTION_12_BIT);  

    bc_adc_set_event_handler(BC_ADC_CHANNEL_A0, adc_event_handler, NULL);
    bc_adc_oversampling_set(BC_ADC_CHANNEL_A0, BC_ADC_OVERSAMPLING_32);
    bc_adc_resolution_set(BC_ADC_CHANNEL_A0, BC_ADC_RESOLUTION_12_BIT);  

    bc_servo_init(&vertical, BC_PWM_P3);
    bc_servo_init(&horizontal, BC_PWM_P1); 
    
    bc_radio_pairing_request("adc", "v1.1.0");
}

void application_task(void)
{
    bc_adc_async_measure(BC_ADC_CHANNEL_A5);
    bc_adc_async_measure(BC_ADC_CHANNEL_A4);
    bc_adc_async_measure(BC_ADC_CHANNEL_A2);
    bc_adc_async_measure(BC_ADC_CHANNEL_A0);
    bc_log_debug("1 = %05u, 2 = %05u, 3 = %05u, 4 = %05u", ldr1, ldr2, ldr3, ldr4);

    if (ldr1 < ldr4){
        ver = ver - 1;
        if (ver < 10){
            ver = 10;
        }
    }
    else if (ldr1 > ldr4)
    {
        ver = ver + 1;
        if (ver > 150){
            ver = 150;
        }
    }

    if (ldr1 < ldr2){
        hor = hor - 1;
        if (hor < 10){
            hor = 10;
        }
    }
    else if (ldr1 > ldr2)
    {
        hor = hor + 1;
        if (hor > 170){
            hor = 170;
        }
    }
    bc_log_debug("vert = %03u, hori = %03u", ver, hor);
    bc_servo_set_angle(&vertical, ver);
    bc_servo_set_angle(&horizontal, hor);
    bc_scheduler_plan_current_from_now(100);
}
