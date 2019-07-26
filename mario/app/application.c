#include <application.h>

float C2 = 7.5;
float D2 = 6.7;
float E2 = 6;
float F2 = 5.75;
float G2 = 5;
float A2 = 4.5;
float H2 = 4;
float C3 = 3.7;
float D3 = 3.3;
float E3 = 3;
float F3 = 2.8;
float G3 = 2.5;
float A3 = 2.2;
float H3 = 2;
float C4 = 1.9;
float non = 0.01;

void delay(uint64_t msec);

void melody(float note,uint16_t duration);


void application_init(void)
{
    bc_pwm_init(BC_PWM_P8);
    bc_pwm_set(BC_PWM_P8, 127);
    bc_pwm_enable(BC_PWM_P8);
}

void application_task()
{  
    while(1)
    {
        melody(A2 , 500);
        melody(A2 , 500);
        melody(A2 , 1000);
        melody(A2 , 500);
        melody(A2 , 500);
        melody(A2 , 1000);
        melody(A2 , 500);
        melody(D3 , 500);
        melody(A2 , 750);
        melody(G2 , 250);
        melody(A2 , 1000);
        melody(C3 , 500);
        melody(C3 , 500);
        melody(C3 , 750);
        melody(C3 , 250);
        melody(C3 , 500);
        melody(H2 , 500);
        melody(H2 , 500);
        melody(H2 , 250);
        melody(H2 , 250);
        melody(H2 , 500);
        melody(A2 , 500);
        melody(A2 , 500);
        melody(H2 , 500);
        melody(A2 , 1000);
        melody(D3 , 1000);
        melody(A2 , 500);
        melody(A2 , 500);
        melody(A2 , 1000);
        melody(A2 , 500);
        melody(A2 , 500);
        melody(A2 , 1000);
        melody(A2 , 500);
        melody(D3 , 500);
        melody(A2 , 750);
        melody(G2 , 250);
        melody(A2 , 1000);
        melody(C3 , 500);
        melody(C3 , 500);
        melody(C3 , 750);
        melody(C3 , 250);
        melody(C3 , 500);
        melody(H2 , 500);
        melody(H2 , 500);
        melody(H2 , 250);
        melody(H2 , 250);
        melody(D3 , 500);
        melody(D3 , 500);
        melody(C3 , 500);
        melody(A2 , 500);
        melody(G2 , 1000);
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
    delay(30);
}