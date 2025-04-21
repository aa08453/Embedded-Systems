#include "../inc/IR.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ir, CONFIG_LOG_DEFAULT_LEVEL);

#define IR_NODE DT_ALIAS(ir)
static const struct gpio_dt_spec ir = GPIO_DT_SPEC_GET(IR_NODE, gpios);

int init_IR(void)
{

    if (!gpio_is_ready_dt(&ir)) 
    {
        LOG_ERR("GPIO device is not ready");
        return -1;
    }

    int ret = gpio_pin_configure_dt(&ir, GPIO_INPUT);
    if (ret < 0) 
    {
        LOG_ERR("Error configuring GPIO pin: %d", ret);
        return -1;
    }
    return 0;
}

int read_IR(void)
{
    int ir_state = gpio_pin_get_dt(&ir);
    return ir_state;
}
