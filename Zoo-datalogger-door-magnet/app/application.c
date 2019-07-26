#include <application.h>
#include <at.h>
#include <bc_ds18b20.h>

#define SEND_DATA_INTERVAL        (15 * 60 * 1000)
#define MEASURE_INTERVAL           (1 * 60 * 1000)

// LED instance
bc_led_t led;
// Button instance
bc_button_t button;
uint16_t click_count = 0;
uint16_t hold_count = 0;
// Lora modem instance
bc_cmwx1zzabz_t lora;
// Thermometer instance
bc_tmp112_t tmp112;
//DS18B20 instance
bc_ds18b20_t ds18d20;


BC_DATA_STREAM_FLOAT_BUFFER(sm_voltage_buffer, 8)
BC_DATA_STREAM_FLOAT_BUFFER(sm_temperature_buffer, (SEND_DATA_INTERVAL / MEASURE_INTERVAL))
BC_DATA_STREAM_FLOAT_BUFFER(sm_temperature_ds18b20_buffer, (SEND_DATA_INTERVAL / MEASURE_INTERVAL))

bc_data_stream_t sm_voltage;
bc_data_stream_t sm_temperature;
bc_data_stream_t sm_temperature_ds18b20;

bc_scheduler_task_id_t battery_measure_task_id;

enum {
    HEADER_BOOT         = 0x00,
    HEADER_UPDATE       = 0x01,
    HEADER_BUTTON_CLICK = 0x02,
    HEADER_BUTTON_HOLD  = 0x03,

} header = HEADER_BOOT;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    if (event == BC_BUTTON_EVENT_CLICK)
    {
        click_count++;

        header = HEADER_BUTTON_CLICK;

        bc_scheduler_plan_now(0);
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        hold_count++;

        header = HEADER_BUTTON_HOLD;

        bc_scheduler_plan_now(0);
    }
}

void tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param)
{
    if (event == BC_TMP112_EVENT_UPDATE)
    {
        float tmp_value = NAN;

        bc_tmp112_get_temperature_celsius(self, &tmp_value);
        bc_data_stream_feed(&sm_temperature, &tmp_value);
    }
}

void handler_ds18b20(bc_ds18b20_t *self, bc_ds18b20_event_t event, void *event_param)
{
    if (event == bc_ds18b20_EVENT_UPDATE)
    {
        float ds_value = NAN;

        if (bc_ds18b20_get_temperature_celsius(self, &ds_value))
        {
           if (ds_value < 80.f)
           {
            bc_data_stream_feed(&sm_temperature_ds18b20, &ds_value);
           }
        }
    }
}

void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    if (event == BC_MODULE_BATTERY_EVENT_UPDATE)
    {
        float voltage = NAN;

        bc_module_battery_get_voltage(&voltage);

        bc_data_stream_feed(&sm_voltage, &voltage);
    }
}

void battery_measure_task(void *param)
{
    if (!bc_module_battery_measure())
    {
        bc_scheduler_plan_current_now();
    }
}


void lora_callback(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_event_t event, void *event_param)
{
    if (event == BC_CMWX1ZZABZ_EVENT_ERROR)
    {
        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_SEND_MESSAGE_START)
    {
        bc_led_set_mode(&led, BC_LED_MODE_ON);

        bc_scheduler_plan_relative(battery_measure_task_id, 20);
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_SEND_MESSAGE_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
    else if (event == BC_CMWX1ZZABZ_EVENT_READY)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
}

bool at_send(void)
{
    bc_scheduler_plan_now(0);

    return true;
}

bool at_status(void)
{
    float value_avg = NAN;

    static const struct {
        bc_data_stream_t *stream;
        const char *name;
        int precision;
    } values[] = {
            {&sm_voltage, "Voltage", 1},
            {&sm_temperature, "Temperature", 1},
            {&sm_temperature_ds18b20, "Temperature DS18B20", 1}
    };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++)
    {
        value_avg = NAN;

        if (bc_data_stream_get_average(values[i].stream, &value_avg))
        {
            bc_atci_printf("$STATUS: \"%s\",%.*f", values[i].name, values[i].precision, value_avg);
        }
        else
        {
            bc_atci_printf("$STATUS: \"%s\",", values[i].name);
        }
    }

    return true;
}

void application_init(void)
{
    bc_data_stream_init(&sm_voltage, 1, &sm_voltage_buffer);
    bc_data_stream_init(&sm_temperature, 1, &sm_temperature_buffer);
    bc_data_stream_init(&sm_temperature_ds18b20, 1, &sm_temperature_ds18b20_buffer);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize Thermometer
    bc_tmp112_init(&tmp112, BC_I2C_I2C0, 0x49);
    bc_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    bc_tmp112_set_update_interval(&tmp112, MEASURE_INTERVAL);

    // Initialize Thermometer DS18B20
    bc_ds18b20_init(&ds18d20, BC_DS18B20_RESOLUTION_BITS_12);
    bc_ds18b20_set_event_handler(&ds18d20, handler_ds18b20, NULL);
    bc_ds18b20_set_update_interval(&ds18d20, MEASURE_INTERVAL);

    // Initialize battery
    bc_module_battery_init();
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    battery_measure_task_id = bc_scheduler_register(battery_measure_task, NULL, 2020);

    // Initialize lora module
    bc_cmwx1zzabz_init(&lora, BC_UART_UART1);
    bc_cmwx1zzabz_set_event_handler(&lora, lora_callback, NULL);
    bc_cmwx1zzabz_set_mode(&lora, BC_CMWX1ZZABZ_CONFIG_MODE_ABP);
    bc_cmwx1zzabz_set_class(&lora, BC_CMWX1ZZABZ_CONFIG_CLASS_A);

    // Initialize AT command interface
    at_init(&led, &lora);
    static const bc_atci_command_t commands[] = {
            AT_LORA_COMMANDS,
            {"$SEND", at_send, NULL, NULL, NULL, "Immediately send packet"},
            {"$STATUS", at_status, NULL, NULL, NULL, "Show status"},
            AT_LED_COMMANDS,
            BC_ATCI_COMMAND_CLAC,
            BC_ATCI_COMMAND_HELP
    };
    bc_atci_init(commands, BC_ATCI_COMMANDS_LENGTH(commands));

    bc_scheduler_plan_current_relative(10 * 1000);
}

void application_task(void)
{
    if (!bc_cmwx1zzabz_is_ready(&lora))
    {
        bc_scheduler_plan_current_relative(100);

        return;
    }

    static uint8_t buffer[7];

    memset(buffer, 0xff, sizeof(buffer));

    buffer[0] = header;

    float voltage_avg = NAN;

    bc_data_stream_get_average(&sm_voltage, &voltage_avg);

    if (!isnan(voltage_avg))
    {
        buffer[1] = voltage_avg * 10.f;
    }

    //buffer[2]

    float temperature_avg = NAN;

    bc_data_stream_get_average(&sm_temperature, &temperature_avg);

    if (!isnan(temperature_avg))
    {
        int16_t temperature_i16 = (int16_t) (temperature_avg * 10.f);

        buffer[3] = temperature_i16 >> 8;
        buffer[4] = temperature_i16;
    }

    float temperature_ds18b20_avg = NAN;

    bc_data_stream_get_average(&sm_temperature_ds18b20, &temperature_ds18b20_avg);

    if (!isnan(temperature_ds18b20_avg))
    {
        int16_t temperature_ds18b20_i16 = (int16_t) (temperature_ds18b20_avg * 10.f);

        buffer[5] = temperature_ds18b20_i16 >> 8;
        buffer[6] = temperature_ds18b20_i16;
    }

    bc_cmwx1zzabz_send_message(&lora, buffer, sizeof(buffer));

    static char tmp[sizeof(buffer) * 2 + 1];
    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        sprintf(tmp + i * 2, "%02x", buffer[i]);
    }

    bc_atci_printf("$SEND: %s", tmp);

    header = HEADER_UPDATE;

    bc_scheduler_plan_current_relative(SEND_DATA_INTERVAL);
}
