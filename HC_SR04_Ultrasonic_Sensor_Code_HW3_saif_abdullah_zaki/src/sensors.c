#include <zephyr/kernel.h>
#include "../inc/sensors.h"
#include "../inc/IR.h"
#include "../inc/US.h"

struct k_thread sensors;

// Function to initialize the sensors
void init_sensors()
{
    init_IR();
    init_US();
}

// Function to read the sensors and return the data
sensors_data_t* read_sensors()
{
    sensors_data_t* data = k_malloc(sizeof(sensors_data_t));  // Allocate memory for the data structure
    if (data == NULL) 
    {
        printk("Memory allocation failed\n");
        return NULL;  // Handle the error if memory allocation fails
    }
    data->IR_data = read_IR();
    data->US_data = read_US();
    return data;
}

// Function to process sensor data (you can implement additional logic here if needed)
sensors_data_t* process_sensor_data(sensors_data_t* data)
{
    return data;
}

// Sensors thread function
void sensors_thread(void *p1, void *p2, void *p3)
{
    int delay_ms = *((int *)p1);  // Retrieve the delay value passed from main

    init_sensors();

    sensors_data_t* sensor_data;

    while (1)
    {   
        sensor_data = read_sensors();

        if (sensor_data != NULL) 
        {
            sensor_data = process_sensor_data(sensor_data);
            k_free(sensor_data);  // Free the allocated memory
        }

        k_sleep(K_MSEC(delay_ms));  // Delay based on user input frequency
    }
}