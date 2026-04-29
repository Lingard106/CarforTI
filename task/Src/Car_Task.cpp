//
// Created by lingard on 2026/4/28.
//

#include "../Inc/Car_Task.h"
#include "bsp_motor.h"


volatile uint32_t g_last_key_time = 0;  // 记录上次有效中断的时间
volatile uint8_t flag = 0;        // 按键按下标志位


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // 判断是否是 PA0 (KEY) 触发的中断
    if (GPIO_Pin == GPIO_PIN_0) {
        uint32_t current_time = HAL_GetTick();

        // ⭐ 中断里只做最基础的时间戳过滤（20ms）
        if (current_time - g_last_key_time > 20) {
            g_last_key_time = current_time;
            flag = 1; // 只置标志位，绝对不加延时！
        }
    }
}

void CarTask::run()
{

    // int8_t station =0;
    for (;;)
    {

        switch (flag)
        {
        case 1:
            // 默认状态（待机或等待）

            break;

        case 2:
            // 其他任务状态
            break;

        default:
            flag = 0;
            break;
        }
        osDelay(5);
    }


}
extern "C" {
    static CarTask CarTask;

    void CarTask_Init()
    {
        CarTask.start((char*)"CarTask",128,osPriorityHigh);
    }

}