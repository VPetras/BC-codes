#include <application.h>
#include <bc_timer.h>
#define INTERVAL (1 * 1 * 1000)
int i = 0;
float C2 = 7.5;
float D2 = 6.7;
float E2 = 6;
float F2 = 5.75;
float G2 = 5;
float A2 = 4.5;
float H2 = 4;
float C3 = 3.7;
float non = 0.01;

void delay(uint64_t msec);

void melody(float note,uint16_t duration);


void application_init(void)
{
    bc_pwm_init(BC_PWM_P8);
    bc_gpio_init(BC_GPIO_P9);
    bc_gpio_set_mode(BC_GPIO_P9, BC_GPIO_MODE_OUTPUT);
    bc_pwm_set(BC_PWM_P8, 127);
    bc_pwm_enable(BC_PWM_P8);
    bc_timer_init();
}

void application_task()
{  
    while(1)
    {
        melody(C2 , 1000);
        melody(E2 , 1000);
        melody(G2 , 1000);
        melody(non , 1000);
        melody(C2 , 1000);
        melody(E2 , 1000);
        melody(G2 , 1000);
        melody(non , 1000);
        melody(E2 , 500);
        melody(E2 , 500);
        melody(D2 , 500);
        melody(E2 , 500);
        melody(F2 , 1000);
        melody(D2 , 1000);
        melody(E2 , 500);
        melody(E2 , 500);
        melody(D2 , 500);
        melody(E2 , 500);
        melody(F2 , 1000);
        melody(D2 , 1000);
        melody(E2 , 1000);
        melody(D2 , 1000);
        melody(C2 , 1000);
        break;
    }
}

void delay (uint64_t msec)
{
    bc_tick_t start_delay = bc_tick_get();
    while (bc_tick_get() < start_delay + msec) 
        ; 
}

void melody(float note,uint16_t duration)
{
    bc_pwm_tim_configure(BC_PWM_TIM3_P6_P7_P8, note , 255);
    delay(duration);
    bc_pwm_tim_configure(BC_PWM_TIM3_P6_P7_P8, non , 255);
    delay(1);
}