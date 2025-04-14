#include <zephyr.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <sys/time_units.h>
#include <device.h>
#include <devicetree.h>
#include <logging/log.h>
#include "hcsr04.h"

LOG_MODULE_REGISTER(hcsr04, CONFIG_SENSOR_LOG_LEVEL);

#define DT_DRV_COMPAT hcsr04

#define TRIG_PIN(node_id) DT_GPIO_PIN(node_id, trig_gpios)
#define ECHO_PIN(node_id) DT_GPIO_PIN(node_id, echo_gpios)
#define TRIG_PORT(node_id) DT_GPIO_LABEL(node_id, trig_gpios)
#define ECHO_PORT(node_id) DT_GPIO_LABEL(node_id, echo_gpios)

#define HCSR04_TIMEOUT_MS 50 // Timeout for ECHO pin transitions

struct hcsr04_data {
    const struct device *trig_dev;
    const struct device *echo_dev;
    int distance; // Measured distance in centimeters
};

static int hcsr04_init(const struct device *dev) {
    struct hcsr04_data *data = dev->data;

    data->trig_dev = device_get_binding(TRIG_PORT(DT_DRV_INST(0)));
    data->echo_dev = device_get_binding(ECHO_PORT(DT_DRV_INST(0)));

    if (!data->trig_dev || !data->echo_dev) {
        LOG_ERR("Could not get GPIO devices");
        return -ENODEV;
    }

    gpio_pin_configure(data->trig_dev, TRIG_PIN(DT_DRV_INST(0)), GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure(data->echo_dev, ECHO_PIN(DT_DRV_INST(0)), GPIO_INPUT);

    LOG_INF("HC-SR04 initialized");
    return 0;
}

static int hcsr04_sample_fetch(const struct device *dev, enum sensor_channel chan) {
    struct hcsr04_data *data = dev->data;
    int64_t start_time, end_time, pulse_duration;

    // Trigger the sensor
    gpio_pin_set(data->trig_dev, TRIG_PIN(DT_DRV_INST(0)), 1);
    k_busy_wait(10); // 10 µs HIGH pulse
    gpio_pin_set(data->trig_dev, TRIG_PIN(DT_DRV_INST(0)), 0);

    // Wait for ECHO pin to go HIGH with timeout
    uint32_t start_wait = k_uptime_get();
    while (gpio_pin_get(data->echo_dev, ECHO_PIN(DT_DRV_INST(0))) == 0) {
        if (k_uptime_get() - start_wait > HCSR04_TIMEOUT_MS) {
            LOG_ERR("Timeout waiting for ECHO pin to go HIGH");
            return -ETIMEDOUT;
        }
        k_yield();
    }
    start_time = k_uptime_get();

    // Wait for ECHO pin to go LOW with timeout
    start_wait = k_uptime_get();
    while (gpio_pin_get(data->echo_dev, ECHO_PIN(DT_DRV_INST(0))) == 1) {
        if (k_uptime_get() - start_wait > HCSR04_TIMEOUT_MS) {
            LOG_ERR("Timeout waiting for ECHO pin to go LOW");
            return -ETIMEDOUT;
        }
        k_yield();
    }
    end_time = k_uptime_get();

    // Calculate pulse duration in microseconds
    pulse_duration = (end_time - start_time) * 1000;
    data->distance = pulse_duration / 58; // Convert to cm
    LOG_DBG("Measured distance: %d cm", data->distance);

    return 0;
}

static int hcsr04_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val) {
    struct hcsr04_data *data = dev->data;

    if (chan != SENSOR_CHAN_DISTANCE) {
        return -ENOTSUP;
    }

    val->val1 = data->distance; // Distance in centimeters
    val->val2 = 0;              // No fractional part
    return 0;
}

static const struct sensor_driver_api hcsr04_driver_api = {
    .sample_fetch = hcsr04_sample_fetch,
    .channel_get = hcsr04_channel_get,
};

#define HCSR04_DEFINE(inst)                                                                    \
    static struct hcsr04_data hcsr04_data_##inst;                                              \
    DEVICE_DT_INST_DEFINE(inst, hcsr04_init, NULL, &hcsr04_data_##inst, NULL, POST_KERNEL,     \
                          CONFIG_SENSOR_INIT_PRIORITY, &hcsr04_driver_api);

DT_INST_FOREACH_STATUS_OKAY(HCSR04_DEFINE)