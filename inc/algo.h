#ifndef ALGO_H
#define ALGO_H

#include "sensors.h"

extern struct k_thread algo;

void compute_command(sensors_data_t *sensor_data, char *command);

void receive_sensors_data(sensors_data_t *sensor_data);

void send_to_motors(char command);

void algo_thread();

#endif