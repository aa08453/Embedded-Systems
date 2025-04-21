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

#define MY_HC_SR04_MM_PER_MS     171

static const uint32_t hw_cycles_per_ms = sys_clock_hw_cycles_per_sec() / 1000;

struct my_hcsr04_data 
{
const struct device *dev;
struct gpio_callback gpio_cb;
struct k_sem sem;
uint32_t start_cycles;
atomic_t echo_gpios_high_cycles;
uint32_t distance_cm;
};

struct my_hcsr04_config 
{
struct gpio_dt_spec trigger_gpios;
struct gpio_dt_spec echo_gpios;
};

static void my_hcsr04_gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

static int my_hcsr04_configure_gpios(const struct my_hcsr04_config *cfg)
{
int ret;

if (!gpio_is_ready_dt(&cfg->trigger_gpios)) {
	LOG_ERR("GPIO '%s' not ready", cfg->trigger_gpios.port->name);
	return -ENODEV;
}
ret = gpio_pin_configure_dt(&cfg->trigger_gpios, GPIO_OUTPUT_LOW);
if (ret < 0) {
	LOG_ERR("Failed to configure '%s' as output: %d", cfg->trigger_gpios.port->name,
		ret);
	return ret;
}

if (!gpio_is_ready_dt(&cfg->echo_gpios)) {
	LOG_ERR("GPIO '%s' not ready", cfg->echo_gpios.port->name);
	return -ENODEV;
}
ret = gpio_pin_configure_dt(&cfg->echo_gpios, GPIO_INPUT);
if (ret < 0) {
	LOG_ERR("Failed to configure '%s' as output: %d", cfg->echo_gpios.port->name, ret);
	return ret;
}

return 0;
}

static int my_hcsr04_configure_interrupt(const struct my_hcsr04_config *cfg, struct my_hcsr04_data *data)
{
int ret;

/* Disable initially to avoid spurious interrupts. */
ret = gpio_pin_interrupt_configure(cfg->echo_gpios.port, cfg->echo_gpios.pin,
	GPIO_INT_DISABLE);
if (ret < 0) {
	LOG_ERR("Failed to configure '%s' as interrupt: %d", cfg->echo_gpios.port->name,
		ret);
	return -EIO;
}
gpio_init_callback(&data->gpio_cb, &my_hcsr04_gpio_callback, BIT(cfg->echo_gpios.pin));
ret = gpio_add_callback(cfg->echo_gpios.port, &data->gpio_cb);
if (ret < 0) {
	LOG_ERR("Failed to add callback on '%s': %d", cfg->echo_gpios.port->name, ret);
	return -EIO;
}
return 0;
}

static int my_hcsr04_init(const struct device *dev)
{
const struct my_hcsr04_config *cfg = dev->config;
struct my_hcsr04_data *data = dev->data;
int ret;

k_sem_init(&data->sem, 0, 1);

ret = my_hcsr04_configure_gpios(cfg);
if (ret < 0) {
	return ret;
}

ret = my_hcsr04_configure_interrupt(cfg, data);
if (ret < 0) {
	return ret;
}
LOG_ERR("Hello",ret);
return 0;
}

static void my_hcsr04_gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
struct my_hcsr04_data *data = CONTAINER_OF(cb, struct my_hcsr04_data, gpio_cb);
const struct my_hcsr04_config *cfg = data->dev->config;

if (gpio_pin_get(dev, cfg->echo_gpios.pin) == 1) {
	data->start_cycles = k_cycle_get_32();
} else {
	atomic_set(&data->echo_gpios_high_cycles, k_cycle_get_32() - data->start_cycles);
	gpio_pin_interrupt_configure_dt(&cfg->echo_gpios, GPIO_INT_DISABLE);
	k_sem_give(&data->sem);
}
}

static int my_hcsr04_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	const struct my_hcsr04_config *cfg = dev->config;
	struct my_hcsr04_data *data = dev->data;

	// Send 10us pulse
	gpio_pin_set_dt(&cfg->trigger_gpios, 1);
	k_busy_wait(10);
	gpio_pin_set_dt(&cfg->trigger_gpios, 0);

	// Wait for echo_gpios pin to go high
	int timeout = 10000;
	while (gpio_pin_get_dt(&cfg->echo_gpios) == 0 && timeout-- > 0) {
		k_busy_wait(1);
	}
	if (timeout <= 0) {
		LOG_ERR("Timeout waiting for echo_gpios to go high");
		return -EIO;
}

uint32_t start_time = k_cycle_get_32();

// Wait for echo_gpios pin to go low
timeout = 10000;
while (gpio_pin_get_dt(&cfg->echo_gpios) == 1 && timeout-- > 0) {
	k_busy_wait(1);
}
if (timeout <= 0) {
	LOG_ERR("Timeout waiting for echo_gpios to go low");
	return -EIO;
}

uint32_t stop_time = k_cycle_get_32();
uint32_t pulse_us = k_cyc_to_us_ceil32(stop_time - start_time);

data->distance_cm = pulse_us / 58;
return 0;
}

static int my_hcsr04_channel_get(const struct device *dev, enum sensor_channel chan,
	struct sensor_value *val)
{
	const struct my_hcsr04_data *data = dev->data;
	uint32_t distance_mm;

	if (chan != SENSOR_CHAN_DISTANCE) {
		return -ENOTSUP;
	}

	distance_mm = MY_HC_SR04_MM_PER_MS * atomic_get(&data->echo_gpios_high_cycles) /
			hw_cycles_per_ms;
	return sensor_value_from_milli(val, distance_mm);
}

static DEVICE_API(sensor, my_hcsr04_driver_api) = {
	.sample_fetch = my_hcsr04_sample_fetch,
	.channel_get = my_hcsr04_channel_get
};


#define MY_HC_SR04_INIT(index)                                                           \
	static struct my_hcsr04_data my_hcsr04_data_##index = {                             \
		.dev = DEVICE_DT_INST_GET(index),                                     \
		.start_cycles = 0,                                                    \
		.echo_gpios_high_cycles = ATOMIC_INIT(0),                                   \
	};                                                                            \
	static struct my_hcsr04_config my_hcsr04_config_##index = {                         \
		.trigger_gpios = GPIO_DT_SPEC_INST_GET(index, trigger_gpios),         \
		.echo_gpios = GPIO_DT_SPEC_INST_GET(index, echo_gpios),               \
	};                                                                            \
																					\
	SENSOR_DEVICE_DT_INST_DEFINE(index, &my_hcsr04_init, NULL, &my_hcsr04_data_##index, \
				&my_hcsr04_config_##index, POST_KERNEL,                  \
				CONFIG_SENSOR_INIT_PRIORITY, &my_hcsr04_driver_api);     \

DT_INST_FOREACH_STATUS_OKAY(MY_HC_SR04_INIT)