#include "bcl.h"

bc_module_relay_t relay;
bc_button_t button;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_module_relay_pulse(&relay, true, 1500);
        bc_module_relay_toggle(&relay);
    }
}

void application_init(void)
{
    bc_module_relay_init(&relay, 0x3B);
    bc_module_relay_set_state(&relay, false);

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);
}