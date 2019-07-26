#include <application.h>

uint16_t C1 = 262;
uint16_t D1b = 277;
uint16_t D1 = 294;
uint16_t E1b = 311;
uint16_t E1 = 330;
uint16_t F1 = 349;
uint16_t G1b = 370;
uint16_t G1 = 392;
uint16_t A1b = 416;
uint16_t A1 = 440;
uint16_t B1b = 466;
uint16_t B1 = 494;
uint16_t C2 = 523;
uint16_t D2b = 554;
uint16_t D2 = 587;
uint16_t E2b = 622;
uint16_t E2 = 659;
uint16_t F2 = 698;
uint16_t G2b = 740;
uint16_t G2 = 784;
uint16_t A2b = 831;
uint16_t A2 = 880;
uint16_t B2b = 932;
uint16_t B2 = 988;
uint16_t C3 = 1047;
uint16_t D3b = 1109;
uint16_t D3 = 1175;
uint16_t E3b = 1245;
uint16_t E3 = 1319;
uint16_t F3 = 1397;
uint16_t G3b = 1480;
uint16_t G3 = 1568;
uint16_t A3b = 1661;
uint16_t A3 = 1760;
uint16_t B3b = 1865;
uint16_t B3 = 1976;
uint16_t C4 = 2093;
uint16_t D4b = 2217;
uint16_t D4 = 2349;
uint16_t E4b = 2489;
uint16_t E4 = 2637;
uint16_t F4 = 2794;
uint16_t G4b = 2960;
uint16_t G4 = 3136;
uint16_t A4b = 3322;
uint16_t A4 = 3520;
uint16_t B4b = 3729;
uint16_t B4 = 3951;
uint16_t C5 = 4186;
uint16_t D5b = 4434;
uint16_t D5 = 4698;
uint16_t E5b = 4978;
uint16_t E5 = 5274;
uint16_t F5 = 5588;
uint16_t G5b = 5920;
uint16_t G5 = 6272;
uint16_t A5b = 6644;
uint16_t A5 = 7040;
uint16_t B5b = 7458;
uint16_t B5 = 7902;
uint16_t C6 = 8372;
uint16_t D6b = 8870;
uint16_t D6 = 9396;
uint16_t E6b = 9956;
uint16_t E6 = 10548;
uint16_t F6 = 11176;
uint16_t G6b = 11840;
uint16_t G6 = 12544;
uint16_t A6b = 13288;
uint16_t non = 0;

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
        melody(G2 , 500);
        melody(G2 , 500);
        melody(G2 , 500);
        melody(E2b , 375);
        melody(B2b , 125);

        melody(G2 , 500);
        melody(E2b , 375);
        melody(B2b , 125);
        melody(G2 , 1000);
        melody(D3 , 500);
        melody(D3 , 500);
        melody(D3 , 500);
        melody(E3b , 375);
        melody(B2b , 125);
        melody(G2b, 500);
        melody(E2b , 375);
        melody(B2b , 125);
        melody(G2 , 1000);
        melody(G3 , 500);
        melody(G2 , 375);
        melody(G2 , 125);
        melody(G3 , 500);
        melody(G3 , 375);
        melody(F3 , 125);

        melody(E3 , 125);
        melody(E3b , 125);
        melody(E3b , 125);
        melody(non , 250);
        melody(G2b , 250);
        melody(D3b , 500);
        melody(B2 , 250);
        melody(B2b , 125);







        break;
    }
}

void delay (uint64_t msec)
{
    bc_tick_t start_delay = bc_tick_get();
    while (bc_tick_get() < start_delay + msec) 
        ; 
}

void melody(uint16_t note,uint16_t duration)
{
    float resolution = 1000000.0/(note*255.0);
    bc_pwm_tim_configure(BC_PWM_TIM3_P6_P7_P8, resolution , 255);
    delay(duration);
    bc_pwm_tim_configure(BC_PWM_TIM3_P6_P7_P8, 0.01 , 255);
    delay(30);
}