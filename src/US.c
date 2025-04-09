#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys_clock.h>
#include <limits.h>
#include "../inc/US.h"

#define ECHO_NODE DT_ALIAS(echo)
static const struct gpio_dt_spec echo = GPIO_DT_SPEC_GET(ECHO_NODE, gpios);

#define TRIG_NODE DT_ALIAS(trig)
static const struct gpio_dt_spec trig = GPIO_DT_SPEC_GET(TRIG_NODE, gpios);

int init_US(void)
{

    if (!gpio_is_ready_dt(&echo) || !gpio_is_ready_dt(&trig)) 
    {
        printk("GPIO device is not ready\n");
        return -1;
    }

    int echo_pin = gpio_pin_configure_dt(&echo, GPIO_INPUT);
    if (echo_pin < 0) 
    {
        printk("Error configuring ECHO GPIO pin: %d\n", echo_pin);
        return -1;
    }

    int trig_pin = gpio_pin_configure_dt(&trig, GPIO_OUTPUT);
    if (trig_pin < 0) 
    {
        printk("Error configuring TRIG GPIO pin: %d\n", trig_pin);
        return -1;
    }
    return 0;
}

int read_US(void)
{
    gpio_pin_set_dt(&trig, 1);
    k_busy_wait(10);  
    gpio_pin_set_dt(&trig, 0);

    int timeout = 10000; 

    // Wait for echo pin to go high
    while (gpio_pin_get_dt(&echo) == 0 && timeout > 0) 
    {
        k_busy_wait(1);
        timeout--;
    }

    if (timeout == 0) 
        return -1;  // Timeout occurred

    uint32_t start_time = k_cycle_get_32();
    
    timeout = 10000;  // New timeout for measuring pulse width
    while (gpio_pin_get_dt(&echo) == 1 && timeout > 0) 
    {
        k_busy_wait(1);
        timeout--;
    }

    if (timeout == 0) 
        return 100;  // Timeout occurred

    uint32_t stop_time = k_cycle_get_32();
    uint32_t elapsed_us = k_cyc_to_us_ceil32(stop_time - start_time);

    return elapsed_us / 58;  // Convert to cm
}
