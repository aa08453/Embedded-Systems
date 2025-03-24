#ifndef MOTOR_H
#define MOTOR_H

extern struct k_thread motors;

void init_motors();

void compute_speeds();

void set_speeds();

void recieve_command();

void motors_thread();

#endif



