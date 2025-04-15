#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <stdlib.h>
#include "../inc/uart.h"

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)
static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

int get_user_frequency()
{
    if (!device_is_ready(uart_dev)) 
    {
        printk("UART device not found!\n");
        return 10;
    }

    char input[10] = {0};
    int index = 0, freq = 0;

    // printk("\nEnter frequency (10Hz - 50Hz): ");

    // while (index < (sizeof(input) - 1))
    // {
    //     uint8_t c;
    //     while (uart_poll_in(uart_dev, &c) != 0) 
    //         k_sleep(K_MSEC(10));  // Wait for input

    //     if (c == '\n' || c == '\r')
    //         break;

    //     input[index++] = c;
    //     printk("%c", c);
    // }

    // input[index] = '\0';
    // freq = atoi(input); 

    // if (freq < 10 || freq > 50) 
    // {
    //     printk("\nInvalid frequency! Must be between 10Hz and 50Hz.\n");
    //     return get_user_frequency(); 
    // }
    freq = 50;
    
    return freq;
}
