#include "../inc/algo.h"
#include "../inc/motors.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

// Define message queues
K_MSGQ_DEFINE(motor_queue, sizeof(vector_t), 10, 4);

struct k_thread algo;

// Thresholds
#define US_THRESHOLD 30  // Ultrasonic threshold for speed control (cm)
#define MIN_SPEED 20      // Min speed percentage
#define MAX_SPEED 100     // Max speed percentage

void compute_command(sensors_data_t *sensor_data, vector_t* vector) 
{
    if (sensor_data->IR_data == 1)
        vector->command = 'S';  // Stop motors
    else 
    {
        if (sensor_data->US_data <= US_THRESHOLD)
            vector->speed = MIN_SPEED + ((sensor_data->US_data * (MAX_SPEED - MIN_SPEED)) / US_THRESHOLD);
        else
            vector->speed = MAX_SPEED;

        vector->command = 'F';  // Move forward
    }
}

void receive_sensors_data(sensors_data_t *sensor_data)
{
    if (k_msgq_get(&sensor_queue, sensor_data, K_FOREVER) == 0)
        printk("Received sensor data: IR=%d, US=%d\n", sensor_data->IR_data, sensor_data->US_data);
}

void send_to_motors(vector_t* vector) 
{
    if (k_msgq_put(&motor_queue, vector, K_NO_WAIT) != 0)
        printk("Motor queue full, dropping command\n");
    else
        printk("Sent command: %c speed: %f\n", vector->command, vector->speed);
}

void algo_thread(void) 
{
    sensors_data_t sensor_data;
    vector_t vector; 

    while (1) 
    {
        receive_sensors_data(&sensor_data);
        compute_command(&sensor_data, &vector);
        send_to_motors(&vector);
    }
}
