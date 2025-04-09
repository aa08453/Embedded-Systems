// sensors.c (MODIFIED to use k_timer instead of k_sleep and add periodic timer)

#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include "../inc/sensors.h"
#include "../inc/IR.h"
#include "../inc/US.h"

#define NUM_TAPS   10  
#define BLOCK_SIZE 10  
#define BUFFER_SIZE 10

struct k_thread sensors;

float32_t firCoeffs[NUM_TAPS] = {0.1, 0.1, 0.1, 0.1, 0.1,
    0.1, 0.1, 0.1, 0.1, 0.1};

arm_fir_instance_f32 fir_US;
static float32_t firState_US[NUM_TAPS + BLOCK_SIZE - 1] = {0};

struct ring_buf us_data_buffer;
uint8_t us_data_storage[BUFFER_SIZE * sizeof(float32_t)];

K_MSGQ_DEFINE(sensor_queue, sizeof(sensors_data_t), 10, 4);
K_SEM_DEFINE(sensor_read_sem, 0, 1);  // Semaphore to be triggered by timer

K_TIMER_DEFINE(sensor_timer, sensor_timer_handler, NULL);

void sensor_timer_handler(struct k_timer *dummy) 
{
    k_work_submit_to_queue(&my_work_q, &sensor_work); // Submit work
}


int init_sensors() 
{
    if (init_IR() != 0 || init_US() != 0)
        return -1; // Initialization failed

    return 0; // Success
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

float32_t process_sensor_data(float32_t *US_array, int len)
{
    static float32_t US_filtered[BUFFER_SIZE];  // Fixed-size array
    arm_fir_f32(&fir_US, US_array, US_filtered, len);
    return US_filtered[len - 1];
}

void init_sensor_buffer() {
    ring_buf_init(&us_data_buffer, BUFFER_SIZE * sizeof(float32_t), us_data_storage);
}

void store_us_data_in_buffer(float32_t us_data)
{
    int ret = ring_buf_put(&us_data_buffer, (uint8_t *)&us_data, sizeof(float32_t));
    if (ret != sizeof(float32_t)) 
    {
        printk("Ring buffer is full. Overwriting oldest data.\n");
        ring_buf_reset(&us_data_buffer);  // Reset buffer to overwrite old data
        ring_buf_put(&us_data_buffer, (uint8_t *)&us_data, sizeof(float32_t));
    }
}

int read_us_data_from_buffer(float32_t *us_data_array, int max_size)
{
    int len = ring_buf_get(&us_data_buffer, (uint8_t *)us_data_array, sizeof(float32_t) * max_size);
    return len / sizeof(float32_t);
}

void sensor_work_handler(struct k_work *work) 
{
    sensors_data_t* sensor_data = read_sensors();
    if (sensor_data == NULL) {
        printk("Failed to allocate memory for sensor data\n");
        return;
    }

    store_us_data_in_buffer(sensor_data->US_data);
    int len = read_us_data_from_buffer(US_array, BUFFER_SIZE);

    if (len > 0) 
    {
        float32_t filtered_us_data = process_sensor_data(US_array, len);
        sensor_data->US_data = (int)filtered_us_data;
        printk("IR_value: %d Filtered US value: %f\n", sensor_data->IR_data, filtered_us_data);
    }

    if (k_msgq_put(&sensor_queue, sensor_data, K_NO_WAIT) != 0) {
        printk("Sensor queue full, dropping data\n");
        k_free(sensor_data);
    }
}

K_THREAD_STACK_DEFINE(work_q_stack, 2048);
struct k_work_q my_work_q;
K_WORK_DEFINE(sensor_work, sensor_work_handler);


void sensors_thread(void *p1, void *p2, void *p3)
{
    // Initialize work queue
    k_work_queue_start(
        &my_work_q,
        work_q_stack,
        K_THREAD_STACK_SIZEOF(work_q_stack),
        5, // Priority
        NULL
    );

    init_sensors();
    init_sensor_buffer();
    arm_fir_init_f32(&fir_US, NUM_TAPS, firCoeffs, firState_US, BLOCK_SIZE);

    k_timer_start(&sensor_timer, K_NO_WAIT, K_MSEC(100));

    // This thread can now perform other tasks or sleep
    while (1) 
    {
        k_sleep(K_FOREVER);
    }
}