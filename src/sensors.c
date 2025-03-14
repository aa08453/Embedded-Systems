#include <zephyr/kernel.h>
#include "arm_math.h"
#include "../inc/sensors.h"
#include "../inc/IR.h"
#include "../inc/US.h"

#define NUM_TAPS   10  
#define BLOCK_SIZE 10  

struct k_thread sensors;

q31_t firCoeffs[NUM_TAPS] = {0x0CCCCCCD, 0x0CCCCCCD, 0x0CCCCCCD, 0x0CCCCCCD, 0x0CCCCCCD,
    0x0CCCCCCD, 0x0CCCCCCD, 0x0CCCCCCD, 0x0CCCCCCD, 0x0CCCCCCD};

arm_fir_instance_q31 fir_US;

static q31_t firState_US[NUM_TAPS + BLOCK_SIZE - 1] = {0};

void init_sensors()
{
    init_IR();
    init_US();
}

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

void process_sensor_arrays(q31_t US_array[], q31_t IR_value)
{
    q31_t US_filtered[BLOCK_SIZE];

    arm_fir_q31(&fir_US, US_array, US_filtered, BLOCK_SIZE);

    int US_filtered_int = US_filtered[BLOCK_SIZE - 1] / (1 << 24); 

    if (US_filtered_int < 0) US_filtered_int = 0;

    printk("IR value: %d, Filtered US value: %d\n", IR_value, US_filtered_int);
}

void process_sensor_data(sensors_data_t* data)
{
    static q31_t US_array[BLOCK_SIZE] = {0};  
    static int index = 0;                     

    US_array[index] = ((q31_t)data->US_data * (1 << 24));  

    index = (index + 1) % BLOCK_SIZE;

    process_sensor_arrays(US_array, data->IR_data);
}

void sensors_thread(void *p1, void *p2, void *p3)
{
    int delay_ms = *((int *)p1);

    init_sensors();

    arm_fir_init_q31(&fir_US, NUM_TAPS, firCoeffs, firState_US, BLOCK_SIZE);

    while (1)
    {   
        sensors_data_t* sensor_data = read_sensors();

        if (sensor_data != NULL) 
        {
            process_sensor_data(sensor_data);
            k_free(sensor_data);
        }

        k_sleep(K_MSEC(delay_ms));
    }
}
