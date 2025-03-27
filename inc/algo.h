#ifndef ALGO_H
#define ALGO_H

#include "sensors.h"

extern struct k_thread algo;

typedef struct
{
    char command;
    double speed;
} vector_t;


void compute_command(sensors_data_t* sensor_data, vector_t* vector);

void receive_sensors_data(sensors_data_t* sensor_data);

void send_to_motors(vector_t* vector);

void algo_thread();

#endif