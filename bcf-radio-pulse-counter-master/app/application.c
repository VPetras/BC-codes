#include <application.h>
#include <bc_eeprom.h>
#include <bc_spi.h>
#include <bc_dice.h>

#define REPORT_INTERVAL (5 * 60 * 1000)
#define REPORT_INTERVAL_INITIAL (1 * 60 * 1000)

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

#define TEMPERATURE_PUB_NO_CHANGE_INTEVAL (15 * 60 * 1000)
#define TEMPERATURE_PUB_VALUE_CHANGE 0.2f
#define TEMPERATURE_UPDATE_INTERVAL (1 * 1000)

#define COLOR_BLACK true

#if ROTATE_SUPPORT
bc_lis2dh12_t lis2dh12;
bc_dice_t dice;
bc_dice_face_t face = BC_DICE_FACE_UNKNOWN;
bc_module_lcd_rotation_t rotation = BC_MODULE_LCD_ROTATION_0;
#endif

#if CORE_R == 2
bc_module_lcd_rotation_t face_2_lcd_rotation_lut[7] =
{
    [BC_DICE_FACE_2] = BC_MODULE_LCD_ROTATION_270,
    [BC_DICE_FACE_3] = BC_MODULE_LCD_ROTATION_180,
    [BC_DICE_FACE_4] = BC_MODULE_LCD_ROTATION_0,
    [BC_DICE_FACE_5] = BC_MODULE_LCD_ROTATION_90
};
#else
bc_module_lcd_rotation_t face_2_lcd_rotation_lut[7] =
{
    [BC_DICE_FACE_2] = BC_MODULE_LCD_ROTATION_90,
    [BC_DICE_FACE_3] = BC_MODULE_LCD_ROTATION_0,
    [BC_DICE_FACE_4] = BC_MODULE_LCD_ROTATION_180,
    [BC_DICE_FACE_5] = BC_MODULE_LCD_ROTATION_270
};
#endif

#if ROTATE_SUPPORT
void lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {
        bc_lis2dh12_result_g_t result;

        bc_lis2dh12_get_result_g(self, &result);

        bc_dice_feed_vectors(&dice, result.x_axis, result.y_axis, result.z_axis);

        face = bc_dice_get_face(&dice);

        if (face > BC_DICE_FACE_1 && face < BC_DICE_FACE_6)
        {
            rotation = face_2_lcd_rotation_lut[face];

            bc_scheduler_plan_now(APPLICATION_TASK_ID);
        }
    }
}
#endif


typedef struct
{
    uint8_t channel;
    float value;
    bc_tick_t next_pub;

} event_param_t;

static event_param_t temperature_event_param = { .next_pub = 0 };

static struct
{
    bc_led_t led;
    bc_button_t button;
    float temperature;
    bc_tag_temperature_t temperature_tag;
    bc_button_t button_channel_b;
    uint32_t button_channel_b_count;

} app;



void pulse_counter_event_handler(bc_module_sensor_channel_t channel, bc_pulse_counter_event_t event, void *event_param);

void battery_event_handler(bc_module_battery_event_t event, void *event_param);

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param);

void button_channel_b_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

void application_init(void)
{
	bc_led_init(&app.led, BC_GPIO_LED, false, false);
	bc_led_set_mode(&app.led, BC_LED_MODE_OFF);

	bc_pulse_counter_init(BC_MODULE_SENSOR_CHANNEL_A, BC_PULSE_COUNTER_EDGE_FALL);
	bc_pulse_counter_set_event_handler(BC_MODULE_SENSOR_CHANNEL_A, pulse_counter_event_handler, NULL);

    bc_module_sensor_set_mode(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_MODE_INPUT);
    bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_PULL_NONE);
	bc_button_init(&app.button_channel_b, BC_GPIO_P5, BC_GPIO_PULL_UP, 1);
	bc_button_set_event_handler(&app.button_channel_b, button_channel_b_event_handler, NULL);

    // Initialize temperature
    temperature_event_param.channel = BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE;
    bc_tag_temperature_init(&app.temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&app.temperature_tag, TEMPERATURE_UPDATE_INTERVAL);
    bc_tag_temperature_set_event_handler(&app.temperature_tag, temperature_tag_event_handler, &temperature_event_param);

    // Initialize battery
    bc_module_battery_init(BC_MODULE_BATTERY_FORMAT_MINI);
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    bc_button_init(&app.button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&app.button, button_event_handler, NULL);

    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);

    bc_scheduler_plan_relative(0, 3000);

    #if ROTATE_SUPPORT
    // Initialize Accelerometer
    bc_dice_init(&dice, BC_DICE_FACE_UNKNOWN);
    bc_lis2dh12_init(&lis2dh12, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_update_interval(&lis2dh12, 5 * 1000);
    bc_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);
    #endif


    bc_radio_pairing_request("Smart-electricity-meter", VERSION);

    bc_led_pulse(&app.led, 2000);

    // Initialize LCD
    bc_module_lcd_init();

}

void application_task(void *param)
{
	(void) param;

    uint32_t channel_count_a;

    channel_count_a = bc_pulse_counter_get(BC_MODULE_SENSOR_CHANNEL_A);

    bc_radio_pub_uint32("pulse-counter/a/count", &channel_count_a);

    bc_scheduler_plan_current_relative(REPORT_INTERVAL);
}

void pulse_counter_event_handler(bc_module_sensor_channel_t channel, bc_pulse_counter_event_t event, void *event_param)
{
    (void) channel;
    (void) event_param;

    if (event == BC_PULSE_COUNTER_EVENT_OVERFLOW)
    {
        // TODO
    }
}

void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    float voltage;

    if (bc_module_battery_get_voltage(&voltage))
    {
        bc_radio_pub_battery(&voltage);
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&app.led, 100);

        static uint16_t event_count = 0;

        event_count++;

        bc_radio_pub_push_button(&event_count);
    }
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event == BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (bc_tag_temperature_get_temperature_celsius(self, &value))
        {
            if ((fabs(value - param->value) >= TEMPERATURE_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_temperature(param->channel, &value);

                param->value = value;
                param->next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
}

{
    static char str_temperature[10];

    if (!bc_module_lcd_is_ready())
    {
    	return;
    }

    bc_system_pll_enable();

#if ROTATE_SUPPORT
    bc_module_lcd_set_rotation(rotation);
#endif

    bc_module_lcd_clear();

    bc_module_lcd_set_font(&bc_font_ubuntu_33);
    snprintf(str_temperature, sizeof(str_temperature), "%.1f   ", temperature_event_param.value);
    int x = bc_module_lcd_draw_string(20, 20, str_temperature, COLOR_BLACK);
    temperature_on_display = temperature_event_param.value;

    bc_module_lcd_set_font(&bc_font_ubuntu_24);
    bc_module_lcd_draw_string(x - 20, 25, "\xb0" "C   ", COLOR_BLACK);

    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(10, 80, "CONSUMPTIOM", COLOR_BLACK);

    snprintf(str_temperature, sizeof(str_temperature), "%.1f \xb0" "C", channel_count_a.value);
    bc_module_lcd_draw_string(40, 100, str_temperature, COLOR_BLACK);

    bc_module_lcd_update();

    bc_system_pll_disable();
}
