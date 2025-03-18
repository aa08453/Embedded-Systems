#include <zephyr/kernel.h>
#include "../inc/sensors.h"
#include "../inc/uart.h"

#define SENSORS_THREAD_STACK_SIZE 4096
#define SENSORS_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensors_stack, SENSORS_THREAD_STACK_SIZE);

void init_threads(void *p)
{
    int delay_ms = *((int *)p); 
    k_thread_create(&sensors, sensors_stack, SENSORS_THREAD_STACK_SIZE, 
                    (k_thread_entry_t) sensors_thread, &delay_ms, NULL, NULL, 
                    SENSORS_THREAD_PRIORITY, 0, K_NO_WAIT);
}


int main(void)
{
    int frequency = get_user_frequency();
    int delay_ms = 1000 / frequency;
    
    printk("\nFrequency set to %dHz, delay set to %d ms\n", frequency, delay_ms);
    
    init_threads(&delay_ms); 

    return 0;
}
