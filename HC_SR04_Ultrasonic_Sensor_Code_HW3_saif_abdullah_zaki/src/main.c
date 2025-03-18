#include <zephyr/kernel.h>
#include <zephyr/console/console.h>
#include <string.h>
#include "../inc/sensors.h"

#define SENSORS_THREAD_STACK_SIZE 1024
#define SENSORS_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensors_stack, SENSORS_THREAD_STACK_SIZE);

int frequency; 

void init_threads(void *p1, void *p2, void *p3)
{
    int delay_ms = *((int *)p1); 
    k_thread_create(&sensors, sensors_stack, SENSORS_THREAD_STACK_SIZE, 
                    (k_thread_entry_t) sensors_thread, &delay_ms, NULL, NULL, 
                    SENSORS_THREAD_PRIORITY, 0, K_NO_WAIT);
}

int get_frequency_input()
{
    int frequency = 0;
    int input_char;
    char input_buffer[10];
    int i = 0;

    printk("Please enter a frequency between 10Hz and 50Hz: ");
    
    // Clear buffer before use
    memset(input_buffer, 0, sizeof(input_buffer));

    // Read input from UART (character by character)
    while (1) {
        input_char = getchar();  // Get one character from user input

        if (input_char == '\n' || input_char == '\r') {
            // Ignore carriage return and new line
            continue;
        }

        if (input_char >= '0' && input_char <= '9') {
            input_buffer[i++] = input_char;  // Add the digit to the buffer
            if (i >= sizeof(input_buffer) - 1) break;  // Prevent buffer overflow
        } else {
            printk("Invalid character! Only digits are allowed.\n");
            while (getchar() != '\n');  // Clear remaining input
            break;
        }

        // If the input is complete and we have a valid number, break the loop
        if (input_char == '\n') {
            break;
        }
    }

    // Convert input buffer to integer
    if (sscanf(input_buffer, "%d", &frequency) == 1 && frequency >= 10 && frequency <= 50) {
        printk("Valid frequency: %dHz\n", frequency);
    } else {
        printk("Invalid frequency. Please enter a value between 10Hz and 50Hz.\n");
    }

    return frequency;
}



int main(void)
{
    frequency = get_frequency_input();  // Get frequency input from the user
    int delay_ms = 1000 / frequency;        // Calculate delay in milliseconds
    
    printk("Frequency set to %dHz, delay set to %d ms\n", frequency, delay_ms);
    
    init_threads(&delay_ms, NULL, NULL);  // Pass delay_ms to the sensor thread

    return 0;
}
