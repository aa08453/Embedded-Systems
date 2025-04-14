#ifndef ZEPHYR_DRIVERS_SENSOR_HCSR04_H_
#define ZEPHYR_DRIVERS_SENSOR_HCSR04_H_

#include <zephyr/drivers/sensor.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the HC-SR04 sensor.
 *
 * This function initializes the GPIO pins and configures the sensor for use.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @return 0 on success, negative error code on failure.
 */
int hcsr04_init(const struct device *dev);

/**
 * @brief Fetch a sample from the HC-SR04 sensor.
 *
 * This function triggers the sensor, measures the echo pulse duration,
 * and calculates the distance.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param chan Sensor channel (ignored for HC-SR04).
 * @return 0 on success, negative error code on failure.
 */
int hcsr04_sample_fetch(const struct device *dev, enum sensor_channel chan);

/**
 * @brief Get the measured distance from the HC-SR04 sensor.
 *
 * This function retrieves the distance value stored after sampling.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param chan Sensor channel (must be SENSOR_CHAN_DISTANCE).
 * @param val Pointer to a sensor_value structure to store the result.
 * @return 0 on success, negative error code on failure.
 */
int hcsr04_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_SENSOR_HCSR04_H_ */