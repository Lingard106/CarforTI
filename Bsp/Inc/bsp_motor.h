//
// Created by lingard on 2026/4/22.
//

#ifndef F407_DRAFT_BSP_MOTOR_H
#define F407_DRAFT_BSP_MOTOR_H
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/************************* 硬件参数配置（根据你的电机修改） *************************/
#define ENCODER_LINES_PER_ROUND   385    // 输出轴单圈物理线数（规格书标称）
#define ENCODER_MULTIPLE          4      // CubeMX 4倍频模式（TI1 and TI2）
#define PULSE_PER_ROUND           (ENCODER_LINES_PER_ROUND * ENCODER_MULTIPLE) // 单圈总脉冲=1540

#define SPEED_SAMPLE_MS     10      // 转速采样周期10ms（100Hz）
#define RPM_FACTOR          (60.0f / (PULSE_PER_ROUND * SPEED_SAMPLE_MS * 0.001f)) // 转速换算系数

#define PWM_MAX_DUTY        49      // 对应定时器ARR=49，满占空比
#define PWM_MAX_SPEED       1000    // 速度输入范围-1000~1000

#define BAT_VOLTAGE_SCALE   2       // 电池分压比（比如10k+10k分压，比例为2，按你的硬件修改）

/************************* 硬件引脚与定时器映射 *************************/
// 电机1
#define M1_PWM_TIM          &htim3
#define M1_PWM_CH           TIM_CHANNEL_2
#define M1_IN1_PORT         GPIOF
#define M1_IN1_PIN          GPIO_PIN_6
#define M1_IN2_PORT         GPIOF
#define M1_IN2_PIN          GPIO_PIN_8
#define M1_ENC_TIM          &htim1

// 电机2
#define M2_PWM_TIM          &htim3
#define M2_PWM_CH           TIM_CHANNEL_4
#define M2_IN1_PORT         GPIOB
#define M2_IN1_PIN          GPIO_PIN_12
#define M2_IN2_PORT         GPIOF
#define M2_IN2_PIN          GPIO_PIN_7
#define M2_ENC_TIM          &htim4

// 驱动芯片使能引脚
#define MOTOR_STBY_PORT     GPIOA
#define MOTOR_STBY_PIN      GPIO_PIN_2

// 电池ADC
#define BAT_ADC_HANDLE      &hadc1

/************************* 电机状态结构体 *************************/
typedef struct {
    volatile int32_t total_pulse;
    volatile int16_t speed_rpm;
    int32_t last_pulse;
    bool inited;
} Motor_State_t;

/************************* 关键修复：extern "C" 包裹函数声明 *************************/
#ifdef __cplusplus
extern "C" {
#endif

    /************************* 外部变量与函数声明 *************************/
    extern Motor_State_t motor1;
    extern Motor_State_t motor2;

    void BSP_Motor_Init(void);
    void BSP_Motor_SetSpeed1(int16_t speed);
    void BSP_Motor_SetSpeed2(int16_t speed);
    uint16_t BSP_Motor_GetBatteryVoltage(void);
    // 线程安全的读取接口（任务中调用时可用临界区保护复合读取）
    int16_t BSP_Motor_GetSpeed1(void);
    int32_t BSP_Motor_GetPulse1(void);
    void BSP_Motor_Lock(void);    // 进入临界区
    void BSP_Motor_Unlock(void);  // 退出临界区

#ifdef __cplusplus
}
#endif
#endif //F407_DRAFT_BSP_MOTOR_H