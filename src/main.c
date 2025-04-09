#include <zephyr/kernel.h>
#include "../inc/sensors.h"
#include "../inc/motors.h"
#include "../inc/algo.h"
#include "../inc/uart.h"

#define SENSORS_THREAD_STACK_SIZE 4096
#define SENSORS_THREAD_PRIORITY   5

#define MOTOR_THREAD_STACK_SIZE   1024
#define MOTOR_THREAD_PRIORITY     5

#define ALGO_THREAD_STACK_SIZE    4096
#define ALGO_THREAD_PRIORITY      5

// Thread stacks
K_THREAD_STACK_DEFINE(sensors_stack, SENSORS_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(motors_stack, MOTOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(algo_stack, ALGO_THREAD_STACK_SIZE);

// Shared mutex (optional, for any shared resources)
K_MUTEX_DEFINE(my_mutex);

void init_threads(int delay_ms) 
{
    k_thread_create(&sensors, sensors_stack, SENSORS_THREAD_STACK_SIZE,
                    (k_thread_entry_t)sensors_thread, &delay_ms, NULL, NULL,
                    SENSORS_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&motors, motors_stack, MOTOR_THREAD_STACK_SIZE,
                    (k_thread_entry_t)motors_thread, NULL, NULL, NULL,
                    MOTOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&algo, algo_stack, ALGO_THREAD_STACK_SIZE,
                    (k_thread_entry_t)algo_thread, NULL, NULL, NULL,
                    ALGO_THREAD_PRIORITY, 0, K_NO_WAIT);
}

int main(void) {
    int frequency = get_user_frequency();  // Custom UART input
    int delay_ms = 1000 / frequency;

    printk("\nFrequency: %d Hz | Delay: %d ms\n", frequency, delay_ms);

    init_threads(&delay_ms);

    return 0;
}
