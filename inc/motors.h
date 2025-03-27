#ifndef MOTOR_H
#define MOTOR_H

#include "algo.h"

extern struct k_thread motors;

void init_motors();

void receive_command(vector_t* vector);

void set_motor_direction(vector_t* vector);

void set_speeds(double duty_cycle);

void motors_thread();

#endif



