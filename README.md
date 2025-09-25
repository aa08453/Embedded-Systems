# Maze Solving Robot using Zephyr RTOS on STM32F3

This project focuses on making a robot capable of solving simple mazes using the wall-following algorithm and the [Waveshare Alpha Bot Kit](https://www.waveshare.com/wiki/AlphaBot) with the [STM32F303 Discovery board](https://docs.zephyrproject.org/latest/boards/st/stm32f3_disco/doc/index.html).

It makes use of Zephyr RTOS and different kernel services like:
- Tasks: one for each aspect of the robot: sensors, algorithm and motors
- Message Queues: inter-task communication,
	- sending sensory data from the two sensors packaged in a struct to the algorithm
	- sending the motor speeds of the two motors in a struct from the algorithm based on the decision to the motors
### Sensing
The sensing module applies a moving average filter of 4 taps on the incoming ultrasonic sensor data using the **CMSIS-DSP** library, which is responsible for detecting the front wall. This is handled by a work handler which is controlled by a separate timer.

The IR sensor is used digitally, and with one placed on the right and is responsible for detecting the side walls.
### Motors and Algorithm
The motors set the speeds based on the decision and speeds sent by the algorithm which implements the basic [wall following algorithm](https://andrewyong7338.medium.com/maze-escape-with-wall-following-algorithm-170c35b88e00). Thresholds and calibrations have been set based on experimentation and need to to be set differently for different bots.
### System Frequency 
The delays of the threads are determined by the chosen frequency of the system in the start which is communicated through UART via the minicom terminal and input by the user.
### Custom Sensor Driver
A custom sensor driver for ultrasonic sensor used (HC -SR04) where a polling based driver was implemented instead of an interrupt based driver already found in Zephyr. Its implementation can be seen in the **[my_hc_sr04](https://github.com/aa08453/Embedded-Systems/tree/sensor-driver/my_hc_sr04)** directory.  

More detail about the system architecture can be found in the [specifications](https://github.com/aa08453/Embedded-Systems/blob/sensor-driver/Block%20Diagram%20and%20Specifications.pdf) document.







