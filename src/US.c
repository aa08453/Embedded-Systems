#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include "../inc/US.h"

struct sensor_value distance;
static const struct device *hcsr04_dev =  DEVICE_DT_GET_ANY(my_hc_sr04);

void init_US(void) {
    
    if (!device_is_ready(hcsr04_dev)) {
        printk("HCSR04 sensor device not ready\n");
        return;
    }
    printk("HCSR04 sensor initialized\n");
}

int read_US(void) {
    if (sensor_sample_fetch(hcsr04_dev) < 0) {
        printk("Failed to fetch sensor sample\n");
        return -1;
    }

    if (sensor_channel_get(hcsr04_dev, SENSOR_CHAN_DISTANCE, &distance) < 0) {
        printk("Failed to get sensor channel data\n");
        return -1;
    }

    return distance.val1; // Distance in cm (integer part)
}
