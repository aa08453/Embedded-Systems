#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>
#include <stdio.h>
#include "../inc/sensors.h"
#include "../inc/IR.h"
#include "../inc/US.h"

#define NUM_TAPS   10  
#define BLOCK_SIZE 10  
#define BUFFER_SIZE 10  // Size of the circular buffer for US_data

struct k_thread sensors;

float32_t firCoeffs[NUM_TAPS] = {0.1, 0.1, 0.1, 0.1, 0.1,
    0.1, 0.1, 0.1, 0.1, 0.1};

arm_fir_instance_f32 fir_US;
static float32_t firState_US[NUM_TAPS + BLOCK_SIZE - 1] = {0};

// Define the ring buffer for storing US_data
struct ring_buf us_data_buffer;
uint8_t us_data_storage[BUFFER_SIZE * sizeof(float32_t)];  // Use uint8_t as required by Zephyr

// Initialize the sensors
void init_sensors()
{
    init_IR();
    init_US();
}

// Read the sensor data
sensors_data_t* read_sensors()
{
    sensors_data_t* data = k_malloc(sizeof(sensors_data_t));
    if (data == NULL) 
    {
        printk("Memory allocation failed\n");
        return NULL;
    }

    data->IR_data = read_IR();  
    data->US_data = read_US(); 
    return data;
}

// Process the sensor data (FIR filter)
float32_t process_sensor_data(float32_t *US_array, int len)
{
    float32_t US_filtered[len];
    arm_fir_f32(&fir_US, US_array, US_filtered, len);
    return US_filtered[len - 1];  // Return the filtered last value
}

// Initialize the ring buffer for US_data
void init_sensor_buffer() {
    ring_buf_init(&us_data_buffer, BUFFER_SIZE * sizeof(float32_t), us_data_storage);
}

// Store US_data in the ring buffer
void store_us_data_in_buffer(float32_t us_data)
{
    int ret = ring_buf_put(&us_data_buffer, (uint8_t *)&us_data, sizeof(float32_t));
    if (ret != sizeof(float32_t)) {
        printk("Ring buffer is full. Could not store US_data\n");
    }
}

// Read the US_data from the ring buffer
int read_us_data_from_buffer(float32_t *us_data_array, int max_size)
{
    int len = ring_buf_get(&us_data_buffer, (uint8_t *)us_data_array, sizeof(float32_t) * max_size);
    return len / sizeof(float32_t);  // Return the number of elements read
}

// The main sensor thread
void sensors_thread(void *p1, void *p2, void *p3)
{
    int delay_ms = *((int *)p1);

    init_sensors();
    init_sensor_buffer();

    arm_fir_init_f32(&fir_US, NUM_TAPS, firCoeffs, firState_US, BLOCK_SIZE);
    float32_t US_array[BUFFER_SIZE] = {0};  // Array for storing the US_data for FIR filtering

    while (1)
    {   
        sensors_data_t* sensor_data = read_sensors();
        
        if (sensor_data != NULL) 
        {
            // Store the US_data in the ring buffer
            store_us_data_in_buffer(sensor_data->US_data);

            // Read the US_data from the buffer
            int len = read_us_data_from_buffer(US_array, BUFFER_SIZE);

            if (len > 0) {
                // If there are enough samples, process them
                float32_t filtered_us_data = process_sensor_data(US_array, len);
                printk("IR_value: %d Filtered US value: %f\n", sensor_data->IR_data, (double)filtered_us_data);
            }

            k_free(sensor_data);  // Free the allocated memory
        }

        k_sleep(K_MSEC(delay_ms));
    }
}
