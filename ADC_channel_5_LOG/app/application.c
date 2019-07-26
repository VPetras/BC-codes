#include <application.h>

// LED instance
bc_led_t led;
bool last_state;


void adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *event_param)
{
    if (event == BC_ADC_EVENT_DONE)
    {
        uint16_t result;

        if (bc_adc_async_get_value(channel, &result))
        {
            bc_log_info("result = %i", result);
            if (result > 10000)
            {
                bc_radio_pub_string("value","high");
                bc_log_debug("high");
                last_state = 0;}
                else
                {
                    if (last_state == 0)
                    {
                    bc_radio_pub_string("value","low");
                    bc_log_debug("low");
                    last_state = 1;
                    }
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

    bc_adc_init();
    bc_adc_set_event_handler(BC_ADC_CHANNEL_A5, adc_event_handler, NULL);
    bc_adc_oversampling_set(BC_ADC_CHANNEL_A5, BC_ADC_OVERSAMPLING_32);
    bc_adc_resolution_set(BC_ADC_CHANNEL_A5, BC_ADC_RESOLUTION_12_BIT);
    
    bc_radio_pairing_request("adc", VERSION);
}

void application_task(void)
{
    bc_adc_async_measure(BC_ADC_CHANNEL_A5);
    bc_scheduler_plan_current_from_now(100);
}
