#include <application.h>

// LED instance
bc_led_t led1;
bc_led_t led2;
bc_led_t led3;
bc_led_t led4;

bc_scheduler_task_id_t id_task_led1;
bc_scheduler_task_id_t id_task_led2;
bc_scheduler_task_id_t id_task_led3;
bc_scheduler_task_id_t id_task_led4;

void toggle_led1();
void toggle_led2();
void toggle_led3();
void toggle_led4();
// Setup the LED
void application_init(void)
{
    bc_led_init(&led1, BC_GPIO_P11, false, false);
    bc_led_init(&led2, BC_GPIO_P13, false, false);
    bc_led_init(&led3, BC_GPIO_P15, false, false);
    bc_led_init(&led4, BC_GPIO_P17, false, false);

    id_task_led1 = bc_scheduler_register(toggle_led1, &led1, BC_TICK_INFINITY);
    id_task_led2 = bc_scheduler_register(toggle_led2, &led2, BC_TICK_INFINITY);
    id_task_ledR = bc_scheduler_register(pwm_R, &ledR, BC_TICK_INFINITY);
    id_task_ledG = bc_scheduler_register(pwm_G, &ledG, BC_TICK_INFINITY);
    id_task_ledB = bc_scheduler_register(pwm_B, &ledB, BC_TICK_INFINITY);

void toggle_led1()
{
    bc_led_set_mode(&led1, BC_LED_MODE_TOGGLE);
    bc_scheduler_plan_current_relative(1500);
}
void toggle_led2()
{
    bc_led_set_mode(&led2, BC_LED_MODE_TOGGLE);
    bc_led_set_mode(&led3, BC_LED_MODE_TOGGLE);
    bc_led_set_mode(&led4, BC_LED_MODE_TOGGLE);
    bc_scheduler_plan_current_relative(2500);
}

void toggle_led3()
{
    bc_led_set_mode(&led3, BC_LED_MODE_TOGGLE);
    bc_scheduler_plan_current_relative(200);
}
void toggle_led4()
{
    bc_led_set_mode(&led4, BC_LED_MODE_TOGGLE);
    bc_scheduler_plan_current_relative(400);
}*/
void application_task(void)
{
    bc_scheduler_plan_now(id_task_led1);
    bc_scheduler_plan_now(id_task_led2);
    //bc_scheduler_plan_now(id_task_led3);
    //bc_scheduler_plan_now(id_task_led4);

}