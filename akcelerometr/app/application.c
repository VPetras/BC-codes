#include <application.h>

bc_lis2dh12_t lis2dh12;
bc_lis2dh12_result_g_t result;
bc_lis2dh12_alarm_t alarm;

void lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_LIS2DH12_EVENT_ALARM)
    {
        bc_log_debug("alarm");
        bc_radio_pub_string("door-sensor","alarm");
    }
    
}
void application_init(void) 
{
    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    alarm.x_high = true;
    alarm.y_high = true;
    alarm.z_high = true;
    alarm.threshold = 2;

    bc_lis2dh12_init(&lis2dh12, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);
    bc_lis2dh12_set_alarm(&lis2dh12, &alarm);

    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);
    bc_radio_pairing_request("Door sensor", VERSION);

}
/*
void application_task(void)
{
    // Logging in action
    bc_log_debug("application_task run");

    // Plan next run this function after 1000 ms
    bc_scheduler_plan_current_from_now(1000);
}*/
