//
// Created by lingard on 2026/4/24.
//

#include "../Inc/motor_Task.h"

#include "bsp_motor.h"
#include "debug_vars.h"
#include "speed_pid.h"



void MotorTask::run()
{
    // SpeedPID speed_pid_left(0.7f, 0.01f, 0.0f, 5000, 5000);
    // SpeedPID speed_pid_right(0.7f, 0.01f, 0.0f, 5000, 5000);

    for (;;)
    {
        BSP_Motor_SetSpeed1(300);
        BSP_Motor_SetSpeed2(-300);
        osDelay(5);
    }


}
extern "C" {
    static MotorTask motorTask;

    void MotorTask_Init()
    {
        motorTask.start((char*)"MotorTask",128,osPriorityHigh);
    }

}