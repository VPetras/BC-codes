#include <application.h>

void delay (uint64_t msec)
{
    bc_tick_t start_delay = bc_tick_get();
    while (bc_tick_get() < start_delay + msec) 
        ; 
}

void application_init(void)
{
bc_gpio_init(BC_GPIO_P8);
bc_gpio_set_mode(BC_GPIO_P8, BC_GPIO_MODE_OUTPUT);
bc_gpio_init(BC_GPIO_P9);
bc_gpio_set_mode(BC_GPIO_P9, BC_GPIO_MODE_OUTPUT);
bc_gpio_init(BC_GPIO_P10);
bc_gpio_set_mode(BC_GPIO_P10, BC_GPIO_MODE_OUTPUT);
bc_gpio_init(BC_GPIO_P11);
bc_gpio_set_mode(BC_GPIO_P11, BC_GPIO_MODE_OUTPUT);
}

void application_task(void)
{
while(1)
{
    bc_gpio_set_output(BC_GPIO_P11, 0);
    bc_gpio_set_output(BC_GPIO_P9, 1);
    delay(1);
    bc_gpio_set_output(BC_GPIO_P8, 0);
    bc_gpio_set_output(BC_GPIO_P10, 1);
    delay(1);
    bc_gpio_set_output(BC_GPIO_P9, 0);
    bc_gpio_set_output(BC_GPIO_P11, 1);
    delay(1);
    bc_gpio_set_output(BC_GPIO_P10, 0);
    bc_gpio_set_output(BC_GPIO_P8, 1);
    delay(1);
}
}
