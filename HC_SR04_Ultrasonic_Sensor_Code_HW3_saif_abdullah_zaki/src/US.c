#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/__assert.h>
#include <string.h>
#include <zephyr/sys_clock.h>
#include <limits.h>
#include "../inc/US.h"

#define ECHO_NODE DT_ALIAS(echo)
static const struct gpio_dt_spec echo = GPIO_DT_SPEC_GET(ECHO_NODE, gpios);

#define TRIG_NODE DT_ALIAS(trig)
static const struct gpio_dt_spec trig = GPIO_DT_SPEC_GET(TRIG_NODE, gpios);


#define MAX_WAIT_CYCLES 1000000

void init_US(void)
{

    if (!gpio_is_ready_dt(&echo) || !gpio_is_ready_dt(&trig)) 
    {
        printk("GPIO device is not ready\n");
        return;
    }

    // Configure the GPIO pin as input for ECHO and output for TRIG
    int echo_pin = gpio_pin_configure_dt(&echo, GPIO_INPUT);
    if (echo_pin < 0) 
    {
        printk("Error configuring ECHO GPIO pin: %d\n", echo_pin);
        return;
    }

    int trig_pin = gpio_pin_configure_dt(&trig, GPIO_OUTPUT);
    if (trig_pin < 0) 
    {
        printk("Error configuring TRIG GPIO pin: %d\n", trig_pin);
        return;
    }
}

int read_US(void)
{
    gpio_pin_set_dt(&trig, 1);
    k_busy_wait(10);  
    gpio_pin_set_dt(&trig, 0);

    uint64_t val = gpio_pin_get_dt(&echo);
    int timeout = 10000; // timeout after 10ms (adjust as necessary)

    // Wait for the echo pin to go high, with a timeout
    while (val == 0 && timeout > 0) {
        val = gpio_pin_get_dt(&echo);
        k_busy_wait(1);  // Small delay to avoid burning CPU
        timeout--;
    }

    if (timeout <= 0) {
        printk("Timeout waiting for echo to go high.\n");
        return -1;
    }

    uint64_t start_time = k_cycle_get_32();
    while (gpio_pin_get_dt(&echo) == 1) {
        // Optionally add another timeout check here
        if (timeout <= 0) {
            printk("Timeout waiting for echo to finish.\n");
            return -1;
        }
        timeout--;
    }

    uint64_t stop_time = k_cycle_get_32();
    uint64_t cycles_spent = stop_time - start_time;

    uint64_t nanoseconds_spent = (cycles_spent * 1000000000UL) / sys_clock_hw_cycles_per_sec();
    uint32_t cm = nanoseconds_spent / 58000; // Convert to centimeters

    printk("Distance: %d cm\n", cm);
    return cm;
}

