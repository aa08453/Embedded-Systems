#ifndef SENSORS_H
#define SENSORS_H

typedef struct
{
    int IR_data;
    int US_data;
} sensors_data_t;

extern struct k_thread sensors;

void init_sensors();

sensors_data_t* read_sensors();

void process_sensor_data(sensors_data_t* data);

void sensors_thread(void *p1, void *p2, void *p3);

#endif