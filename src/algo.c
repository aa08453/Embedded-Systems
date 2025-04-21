#include "../inc/algo.h"
#include "../inc/motors.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(algo, CONFIG_LOG_DEFAULT_LEVEL);

// Define message queues
K_MSGQ_DEFINE(motor_queue, sizeof(vector_t), 10, 4);

struct k_thread algo;

// Thresholds
#define MAX_THRESHOLD 13
#define MIN_THRESHOLD 8
#define MIN_SPEED 20
#define L_MAX_SPEED 100 //100
#define R_MAX_SPEED 90 //98

void compute_command(sensors_data_t *sensor_data, vector_t* vector) 
{
    // int right = (sensor_data->US_data > MAX_THRESHOLD); //away from wall
    // int left =  (sensor_data->US_data < MIN_THRESHOLD); //towards wall

    if (sensor_data->IR_data == 1)
    {   
        vector->speed_l = 0;
        vector->speed_r = 0;
        vector->command = 'F';
        // if (sensor_data->US_data < MAX_THRESHOLD)
        // {
        //     vector->speed_l = 0;//L_MAX_SPEED*0.3;
        //     vector->speed_r = 0;
        // }
        // else{
        //     vector->speed_l = 0;
        //     vector->speed_r = 0;//R_MAX_SPEED*0.3;
        // }
        // k_sleep(K_MSEC(300));

    }
    else
    {   
        // vector->speed_l = L_MAX_SPEED; //65  //sensor_data->US_data - MIN_THRESHOLD;
        // vector->speed_r = R_MAX_SPEED; //75
        if ((sensor_data->US_data > MAX_THRESHOLD))
        {
            vector->speed_l = 0;//L_MAX_SPEED*0.0; // 68
            vector->speed_r = 0;//R_MAX_SPEED*0.0; // 65 //sensor_data->US_data - MIN_THRESHOLD;
        }
        else if ((sensor_data->US_data < MIN_THRESHOLD))
        {
            vector->speed_l = L_MAX_SPEED*0.45; //65  //sensor_data->US_data - MIN_THRESHOLD;
            vector->speed_r = R_MAX_SPEED*0.35; //75
        }
        
    }

    vector->command = 'F';

}

void receive_sensors_data(sensors_data_t *sensor_data)
{
    if (k_msgq_get(&sensor_queue, sensor_data, K_FOREVER) == 0)
        LOG_INF("Received sensor data: IR=%d, US=%f", sensor_data->IR_data,(double)sensor_data->US_data);
}

void send_to_motors(vector_t* vector) 
{
    if (k_msgq_put(&motor_queue, vector, K_NO_WAIT) != 0)
        LOG_INF("Motor queue full, dropping command");
    else
        LOG_INF("Sent command: %c speed_r: %f, speed_l: %f", vector->command, vector->speed_r, vector->speed_l);
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
