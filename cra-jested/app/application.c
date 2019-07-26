#include <application.h>

bc_led_t led1;
bc_led_t led2;
bc_led_t led3;
bc_led_t led4;

bc_scheduler_task_id_t id_task_led1;
bc_scheduler_task_id_t id_task_led2;
bc_scheduler_task_id_t id_task_led3;
bc_scheduler_task_id_t id_task_led4;

bc_scheduler_task_id_t id_task_ledR;
bc_scheduler_task_id_t id_task_ledG;
bc_scheduler_task_id_t id_task_ledB;

void toggle_led1();
void toggle_led2();
void toggle_led3();
void toggle_led4();

void pwm_R();
void pwm_G();
void pwm_B();

uint16_t ledR = 0;
uint16_t ledG = 85;
uint16_t ledB = 170;

uint8_t stateR = 0;
uint8_t stateG = 0;
uint8_t stateB = 0;
uint8_t stateleds = 0;


void application_init(void)
{
    bc_led_init(&led1, BC_GPIO_P11, false, false);
    bc_led_init(&led2, BC_GPIO_P13, false, false);
    bc_led_init(&led3, BC_GPIO_P15, false, false);
    bc_led_init(&led4, BC_GPIO_P17, false, false);

    bc_pwm_init(BC_PWM_P6);
    bc_pwm_set(BC_PWM_P6, 0);
    bc_pwm_enable(BC_PWM_P6);
    bc_pwm_init(BC_PWM_P7);
    bc_pwm_set(BC_PWM_P7, 0);
    bc_pwm_enable(BC_PWM_P7);
    bc_pwm_init(BC_PWM_P8);
    bc_pwm_set(BC_PWM_P8, 0);
    bc_pwm_enable(BC_PWM_P8);

    id_task_led1 = bc_scheduler_register(toggle_led1, &led1, BC_TICK_INFINITY);
    id_task_led2 = bc_scheduler_register(toggle_led2, &led2, BC_TICK_INFINITY);
    id_task_ledR = bc_scheduler_register(pwm_R, &ledR, BC_TICK_INFINITY);
    id_task_ledG = bc_scheduler_register(pwm_G, &ledG, BC_TICK_INFINITY);
    id_task_ledB = bc_scheduler_register(pwm_B, &ledB, BC_TICK_INFINITY);
}

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

void pwm_R()
{
    bc_pwm_set(BC_PWM_P6, ledR);
    if (stateR == 0 && ledR < 200)
    {
        ledR = ledR + 5;
    }
    else if (stateR == 0 && ledR >= 200)
    {
        stateR = 1;
    }
    else if (stateR == 1 && ledR > 0)
    {
        ledR = ledR - 5;
    }
    else if (stateR == 1 && ledR <= 0)
    {
        stateR = 0;
    }
    bc_scheduler_plan_current_relative(75);

}

void pwm_G()
{
    bc_pwm_set(BC_PWM_P7, ledG);
    if (stateG == 0 && ledG < 200)
    {
        ledG = ledG + 5;
    }
    else if (stateG == 0 && ledG >= 200)
    {
        stateG = 1;
    }
    else if (stateG == 1 && ledG > 0)
    {
        ledG = ledG - 5;
    }
    else if (stateG == 1 && ledG <= 0)
    {
        stateG = 0;
    }
    bc_scheduler_plan_current_relative(75);
}

void pwm_B()
{
    bc_pwm_set(BC_PWM_P8, ledB);
    if (stateB == 0 && ledB < 200)
    {
        ledB = ledB + 5;
    }
    else if (stateB == 0 && ledB >= 200)
    {
        stateB = 1;
    }
    else if (stateB == 1 && ledB > 0)
    {
        ledB = ledB - 5;
    }
    else if (stateB == 1 && ledB <= 0)
    {
        stateB = 0;
    }
    bc_scheduler_plan_current_relative(75);
}

void application_task(void)
{
    bc_scheduler_plan_now(id_task_led1);
    bc_scheduler_plan_now(id_task_led2);
    bc_scheduler_plan_now(id_task_ledR);
    bc_scheduler_plan_now(id_task_ledG);
    bc_scheduler_plan_now(id_task_ledB);
}