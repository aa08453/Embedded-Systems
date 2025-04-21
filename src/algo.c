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
#define MAX_THRESHOLD 12
#define MIN_THRESHOLD 8
#define MIN_SPEED 20
#define L_MAX_SPEED 50 //100
#define R_MAX_SPEED 49 //98

void compute_command(sensors_data_t *sensor_data, vector_t* vector) 
{
    if (sensor_data->IR_data == 1)
    {   
        if (sensor_data->US_data < MAX_THRESHOLD)
        {
            vector->speed_l = 0;//L_MAX_SPEED*0.3;
            vector->speed_r = 0;
        }
        else{
            vector->speed_l = 0;
            vector->speed_r = 0;//R_MAX_SPEED*0.3;
        }
        // k_sleep(K_MSEC(300));

    }
    else
    {   
        vector->speed_l = L_MAX_SPEED; //65  //sensor_data->US_data - MIN_THRESHOLD;
        vector->speed_r = R_MAX_SPEED; //75
        // if (sensor_data->US_data > MAX_THRESHOLD)
        // {
        //     vector->speed_l = L_MAX_SPEED*0.65; //65  //sensor_data->US_data - MIN_THRESHOLD;
        //     vector->speed_r = R_MAX_SPEED*0.75; //75
        // }
        // else if ((sensor_data->US_data < MIN_THRESHOLD))
        // {
        //     vector->speed_r = R_MAX_SPEED*0.65; // 65 //sensor_data->US_data - MIN_THRESHOLD;
        //     vector->speed_l = L_MAX_SPEED*0.75; // 68
        // }
        // else{
        //     vector->speed_r = R_MAX_SPEED*0.6;//sensor_data->US_data - MIN_THRESHOLD;
        //     vector->speed_l = L_MAX_SPEED*0.6;
        // }
    }
    // {
    // vector->speed_l = L_MAX_SPEED;
    // vector->speed_r = R_MAX_SPEED;
    // }
    vector->command = 'F';

    // if (sensor_data->US_data < US_THRESHOLD) 
    //         // Too close to the left wall, turn slightly right
    //         vector->command = 'R';  // Turn right
    
    //     // else if (sensor_data->US_data > US_THRESHOLD) 
    //     //     // Too far from the left wall, turn slightly left
    //     //     vector->command = 'L';  // Turn left
        
    //     // else 
    //         // Optimal distance from the left wall, move forward
    //         vector->command = 'F';  // Move forward

        // Adjust speed based on proximity to the left wall

        // if (sensor_data->US_data < US_THRESHOLD) {
        //     // Closer to the wall, reduce speed proportionally
        //     //reducing right motor speed if the robot is appraching the wall on the left
        //     vector->speed_r = MIN_SPEED + ((sensor_data->US_data * (MAX_SPEED - MIN_SPEED)) / US_THRESHOLD);
        //     vector->speed_l = MAX_SPEED;
            
        //     // vector->speed_l = MAX_SPEED+10;
        //     // vector->speed_r = MIN_SPEED;

        // }
        //     // vector->speed_l = MIN_SPEED + ((sensor_data->US_data * (MAX_SPEED - MIN_SPEED)) / US_THRESHOLD);
        // else if (sensor_data->US_data > US_THRESHOLD) {
        //     // Closer to the wall, reduce speed proportionally
        //     //reducing left motor speed if the robot is appraching the wall on the left
        //     vector->speed_l = MIN_SPEED + ((sensor_data->US_data * (MAX_SPEED - MIN_SPEED)) / US_THRESHOLD);
        //     vector->speed_r = MAX_SPEED;


        //     vector->speed_l = MIN_SPEED;
        //     vector->speed_r = MAX_SPEED;

        // }
        // else {
        //     vector->speed_l = MAX_SPEED+10;
        //     vector->speed_r = MAX_SPEED;
        //     // equal to threshold so both speeds are equal
           
        // }
        // vector->speed_l= 100;
        // vector->speed_r = 98;
}

void receive_sensors_data(sensors_data_t *sensor_data)
{
    if (k_msgq_get(&sensor_queue, sensor_data, K_FOREVER) == 0)
        printk("Received sensor data: IR=%d, US=%f\n", sensor_data->IR_data,(double)sensor_data->US_data);
}

void send_to_motors(vector_t* vector) 
{
    if (k_msgq_put(&motor_queue, vector, K_NO_WAIT) != 0)
        printk("Motor queue full, dropping command\n");
    else
        printk("Sent command: %c speed_r: %f, speed_l: %f\n", vector->command, vector->speed_r, vector->speed_l);
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
