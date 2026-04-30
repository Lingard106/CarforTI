#include "../Inc/Car_Task.h"
#include "bsp_motor.h"
#include "tcrt.h"
#include "Balance_Task.h"
#include "speed_pid.h"
#include "stm32f4xx_hal.h"



#define LED_PIN     GPIO_PIN_3
#define LED_PORT    GPIOB
#define BEEP_PIN    GPIO_PIN_6
#define BEEP_PORT   GPIOA

volatile uint32_t g_last_key_time = 0;
volatile uint8_t flag = 0;
extern BalanceControl_t g_balance_ctrl;
extern SpeedPID motor_speed_pid;



// 非阻塞计时变量（修复卡死问题）
// static uint32_t alarm_tick = 0;
// static uint8_t alarm_en = 0;


// 按键PA4中断回调
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_4) {
        uint32_t now = HAL_GetTick();
        if (now - g_last_key_time > 200) {
            g_last_key_time = now;
            if (flag == 0) {flag = 1;motor1.total_pulse=0;}
            else if (flag == 1) flag = 0;
            else if (flag == 2) flag = 0;
            //alarm_en = 0; // 重置声光
        }
    }
}

uint8_t line_states[TCRT_CHANNEL_COUNT]={0};

void CarTask::run() {
    // TCRT_Init();
    //osDelay(50);
    flag = 0;

    for (;;) {
        // 循迹定时采样（无中断，无抖动）
        // TCRT_SampleAll();
        // TCRT_GetAllStates(line_states);

        switch (flag) {
            case 0: // 停止模式
                g_balance_ctrl.target_speed = 0;
                break;

            case 1: // 前进模式（只有压到黑线才停车！）
                {
                    g_balance_ctrl.target_speed = 60; // 温柔速度，不抖动

                    // // 🔥 核心修复：只有真正检测到黑线才触发
                    // uint8_t black = 0;
                    // for (int i = 0; i < TCRT_CHANNEL_COUNT; i++) {
                    //     if (line_states[i] == 1) black = 1;
                    // }

                    if (motor1.total_pulse<=-7500) {
                        g_balance_ctrl.target_speed = 0;
                        // alarm_en = 1;
                        // alarm_tick = HAL_GetTick();
                        motor_speed_pid.Clear();
                        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET);
                        osDelay(80);
                        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET);
                        flag = 2;
                    }
                    break;
                }
            case 2: // 黑线停车模式（声光提示）
                g_balance_ctrl.target_speed = 0;

                    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_SET);
               osDelay(150);
                    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(BEEP_PORT, BEEP_PIN, GPIO_PIN_RESET);
            flag = 0;

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
    void CarTask_Init() {
        CarTask.start((char*)"CarTask", 256, osPriorityRealtime);
    }
}