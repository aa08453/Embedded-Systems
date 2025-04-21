#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include "../inc/US.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(us, CONFIG_LOG_DEFAULT_LEVEL);

struct sensor_value distance;
static const struct device *hcsr04_dev =  DEVICE_DT_GET_ANY(my_hc_sr04);

int init_US(void) 
{    
    if (!device_is_ready(hcsr04_dev)) 
    {
        LOG_ERR("HCSR04 sensor device not ready");
        return -1;
    }
    LOG_INF("HCSR04 sensor initialized");
    return 0;
}

int read_US(void) {
    if (sensor_sample_fetch(hcsr04_dev) < 0) {
        LOG_ERR("Failed to fetch sensor sample");
        return -1;
    }

    if (sensor_channel_get(hcsr04_dev, SENSOR_CHAN_DISTANCE, &distance) < 0) {
        LOG_ERR("Failed to get sensor channel data");
        return -1;
    }

    return distance.val1; // Distance in cm (integer part)
}
