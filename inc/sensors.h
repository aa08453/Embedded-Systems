#ifndef SENSORS_H
#define SENSORS_H

#include "arm_math.h"
#include <zephyr/sys/ring_buffer.h>


typedef struct
{
    int IR_data;
    int US_data;
} sensors_data_t;

extern struct k_thread sensors;
extern struct k_msgq sensor_queue;

void init_sensors();

sensors_data_t* read_sensors();

float32_t process_sensor_data(float32_t *US_array, int BLOCK_SIZE);

void sensors_thread(void *p1, void *p2, void *p3);

#endif