#include "../inc/motors.h"
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

// Motor control thread
struct k_thread motors;

// Define message queue (extern from algo.c)
extern struct k_msgq motor_queue;

// Motor GPIO and PWM definitions
#define IN1 DT_ALIAS(in1)
#define IN2 DT_ALIAS(in2)
#define IN3 DT_ALIAS(in3)
#define IN4 DT_ALIAS(in4)

static const struct pwm_dt_spec enA = PWM_DT_SPEC_GET(DT_ALIAS(ena));
static const struct pwm_dt_spec enB = PWM_DT_SPEC_GET(DT_ALIAS(enb));

static const struct gpio_dt_spec in1 = GPIO_DT_SPEC_GET(IN1, gpios);
static const struct gpio_dt_spec in2 = GPIO_DT_SPEC_GET(IN2, gpios);
static const struct gpio_dt_spec in3 = GPIO_DT_SPEC_GET(IN3, gpios);
static const struct gpio_dt_spec in4 = GPIO_DT_SPEC_GET(IN4, gpios);

#define MAX_PERIOD PWM_MSEC(100U)

uint32_t period = MAX_PERIOD;

int ret1, ret2;


// Motor initialization function
void init_motors(void)
{
    int ret;

    ret = gpio_pin_configure_dt(&in1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to configure in1 pin", ret);            
        return; 
    }
    ret = gpio_pin_configure_dt(&in2, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to configure in2 pin", ret);            
        return;
    }
    ret = gpio_pin_configure_dt(&in3, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to configure in3 pin", ret);            
        return;
    }
    ret = gpio_pin_configure_dt(&in4, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Error %d: failed to configure in4 pin", ret);                          
        return;
    }

    if (!pwm_is_ready_dt(&enA) || !pwm_is_ready_dt(&enB)) {
        LOG_ERR("Error %d: failed to configure enable pin", ret);                    
        return;
    }

    LOG_INF("Motors initialized successfully");
}

// Function to set motor direction based on received command
void set_motor_direction(vector_t* vector)
{
    switch (vector->command) {
        case 'F': // Move Forward
            gpio_pin_set_dt(&in1, 1);
            gpio_pin_set_dt(&in2, 0);
            gpio_pin_set_dt(&in3, 0);
            gpio_pin_set_dt(&in4, 1);
            LOG_INF("Motors moving forward");
            break;

        case 'B': // Move Backward
            gpio_pin_set_dt(&in1, 0);
            gpio_pin_set_dt(&in2, 1);
            gpio_pin_set_dt(&in3, 1);
            gpio_pin_set_dt(&in4, 0);
            LOG_INF("Motors moving backward");
            break;


        default:
            LOG_INF("Motors stopped");
            gpio_pin_set_dt(&in1, 0);
            gpio_pin_set_dt(&in2, 0);
            gpio_pin_set_dt(&in3, 0);
            gpio_pin_set_dt(&in4, 0);
            break;
    }
}

// Function to adjust speed
void set_speeds(double duty_cycle)
{
    int new_speed = duty_cycle * period;
    ret1 = pwm_set_dt(&enA, period, new_speed);
    ret2 = pwm_set_dt(&enB, period, new_speed);

    if (ret1) 
    {
        printk("Error %d: failed to set pulse width for enA\n", ret1);
        return;
    }

    if (ret2) 
    {
        printk("Error %d: failed to set pulse width for enB\n", ret2);
        return;
    }

    printk("Motor speeds updated, duty cycle: %f\n", duty_cycle);
}

// Function to receive motor commands from message queue
void receive_command(vector_t* vector)
{
    if (k_msgq_get(&motor_queue, vector, K_FOREVER) == 0) 
    {
        printk("Received motor command: %c speed: %f\n", vector->command, vector->speed);
    }
}

// Motor thread function
void motors_thread(void)
{
    init_motors();
    vector_t vector;

    while (1) {

        receive_command(&vector);
        set_speeds(vector.speed/100);
        set_motor_direction(&vector);
    }
}
