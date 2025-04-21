/*
* Copyright (c) 2024 Adrien Leravat
*
* SPDX-License-Identifier: Apache-2.0
*/

#define DT_DRV_COMPAT my_hc_sr04

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(MY_HC_SR04, CONFIG_SENSOR_LOG_LEVEL);

struct my_hcsr04_data {
	const struct device *dev;
	uint32_t distance_cm;
};

struct my_hcsr04_config {
	struct gpio_dt_spec trigger_gpios;
	struct gpio_dt_spec echo_gpios;
};


static int my_hcsr04_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	const struct my_hcsr04_config *cfg = dev->config;
	struct my_hcsr04_data *data = dev->data;

	// Send 10us pulse
	gpio_pin_set_dt(&cfg->trigger_gpios, 1);
	k_busy_wait(10);
	gpio_pin_set_dt(&cfg->trigger_gpios, 0);

	// Wait for echo high
	int timeout = 10000;
	while (gpio_pin_get_dt(&cfg->echo_gpios) == 0 && timeout-- > 0) {
		k_busy_wait(1);
	}
	if (timeout <= 0) {
		LOG_ERR("Timeout: waiting for echo HIGH");
		return -EIO;
	}

	uint32_t start_time = k_cycle_get_32();

	// Wait for echo low
	timeout = 30000;
	while (gpio_pin_get_dt(&cfg->echo_gpios) == 1 && timeout-- > 0) {
		k_busy_wait(1);
	}
	if (timeout <= 0) {
		LOG_ERR("Timeout: waiting for echo LOW");
		return -EIO;
	}

	uint32_t stop_time = k_cycle_get_32();
	uint32_t pulse_us = k_cyc_to_us_ceil32(stop_time - start_time);
	data->distance_cm = pulse_us / 58;

	return 0;
}

static int my_hcsr04_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
	const struct my_hcsr04_data *data = dev->data;

	if (chan != SENSOR_CHAN_DISTANCE) {
		return -ENOTSUP;
	}

	val->val1 = data->distance_cm;
	val->val2 = 0;
	return 0;
}

static int my_hcsr04_init(const struct device *dev)
{
	const struct my_hcsr04_config *cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->trigger_gpios) || !gpio_is_ready_dt(&cfg->echo_gpios)) {
		LOG_ERR("GPIOs not ready");
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(&cfg->trigger_gpios, GPIO_OUTPUT_LOW) != 0 ||
		gpio_pin_configure_dt(&cfg->echo_gpios, GPIO_INPUT) != 0) {
		LOG_ERR("Failed to configure GPIOs");
		return -EIO;
	}

	return 0;
}

static const struct sensor_driver_api my_hcsr04_driver_api = {
	.sample_fetch = my_hcsr04_sample_fetch,
	.channel_get = my_hcsr04_channel_get,
};

#define MY_HC_SR04_INIT(index)                                                       \
	static struct my_hcsr04_data my_hcsr04_data_##index;                         \
	static const struct my_hcsr04_config my_hcsr04_config_##index = {           \
		.trigger_gpios = GPIO_DT_SPEC_INST_GET(index, trigger_gpios),       \
		.echo_gpios = GPIO_DT_SPEC_INST_GET(index, echo_gpios),             \
	};                                                                          \
	SENSOR_DEVICE_DT_INST_DEFINE(index,                                        \
		my_hcsr04_init, NULL, &my_hcsr04_data_##index,                        \
		&my_hcsr04_config_##index, POST_KERNEL,                              \
		CONFIG_SENSOR_INIT_PRIORITY, &my_hcsr04_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MY_HC_SR04_INIT)
