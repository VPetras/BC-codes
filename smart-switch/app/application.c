#include <application.h>
#include <bc_servo.h>

bc_servo_t servo_left;
bc_servo_t servo_right;

void servo_left_angle_set(uint64_t *id, const char *topic, void *value, void *param);
void servo_right_angle_set(uint64_t *id, const char *topic, void *value, void *param);


static const bc_radio_sub_t subs[] = {
    // state/set
    {"servo/left/angle/set", BC_RADIO_SUB_PT_INT, servo_left_angle_set, NULL },
    {"servo/right/angle/set", BC_RADIO_SUB_PT_INT, servo_right_angle_set, NULL }
};



void servo_left_angle_set(uint64_t *id, const char *topic, void *value, void *param)
{
    uint8_t angle = *(int*)value;
    bc_servo_set_angle(&servo_left, angle);
}

void servo_right_angle_set(uint64_t *id, const char *topic, void *value, void *param)
{
    uint8_t angle = *(int*)value;
    bc_servo_set_angle(&servo_right, angle);
}



void application_init(void)
{

    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING);
    bc_radio_set_subs((bc_radio_sub_t *) subs, sizeof(subs)/sizeof(bc_radio_sub_t));

    // We cannot use pins P0_P1_P2_P3 because they collide with TIM2 which is used for LEDS
    bc_servo_init(&servo_left, BC_PWM_P6);
    bc_servo_set_angle(&servo_left, 90);
    bc_servo_init(&servo_right, BC_PWM_P12);
    bc_servo_set_angle(&servo_right, 90);

    bc_radio_pairing_request("smart-window-switch", VERSION);
}