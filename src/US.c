#include <zephyr.h>
#include <drivers/sensor.h>
#include <stdio.h>

// Global variable for the HC-SR04 device
static const struct device *hcsr04_dev;

/**
 * @brief Initialize the HC-SR04 sensor.
 *
 * This function initializes the HC-SR04 sensor and checks if it's ready.
 *
 * @return 0 on success, negative error code on failure.
 */
void init(void) 
{
    hcsr04_dev = DEVICE_DT_GET_ANY(hcsr04);

    if (!device_is_ready(hcsr04_dev)) 
    {
        printk("HC-SR04 device not ready\n");
        return; // Return standard error code for "No such device"
    }

    printk("HC-SR04 initialized successfully\n");
    return;
}

/**
 * @brief Read the distance from the HC-SR04 sensor.
 *
 * This function fetches a sample from the HC-SR04 sensor and retrieves the distance.
 *
 * @return 0 on success, negative error code on failure.
 */
int read_US(void) 
{
    struct sensor_value distance;

    // Fetch a sample from the sensor
    int ret = sensor_sample_fetch(hcsr04_dev);
    if (ret < 0) 
    {
        printk("Failed to fetch sample from HC-SR04: %d\n", ret);
        return ret;
    }

    // Get the distance value
    ret = sensor_channel_get(hcsr04_dev, SENSOR_CHAN_DISTANCE, &distance);
    if (ret < 0) 
    {
        printk("Failed to get distance channel: %d\n", ret);
        return 0;
    }

    // Print the measured distance
    printk("Distance: %d cm\n", distance.val1);
    return distance.val1;
}

