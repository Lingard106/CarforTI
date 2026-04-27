//
// Created by lingard on 2026/4/24.
//

#include "../Inc/motor_Task.h"

#include "bsp_motor.h"
#include "debug_vars.h"

void MotorTask::run()
{


    for (;;)
    {
        BSP_Motor_SetSpeed1(300);
        BSP_Motor_SetSpeed2(300);
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