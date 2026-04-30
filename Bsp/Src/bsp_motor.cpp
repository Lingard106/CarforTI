//
// Created by lingard on 2026/4/22.
//

#include "../Inc/bsp_motor.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "adc.h"
#include "bsp_dwt.h"

// FreeRTOS 头文件（目前只用到 taskENTER_CRITICAL）
#include "FreeRTOS.h"
#include "task.h"

/************************* 全局变量 *************************/
Motor_State_t motor1 = {0, 0, 0, false,0,0};   // 按顺序初始化：total_pulse, speed_rpm, last_pulse, inited
Motor_State_t motor2 = {0, 0, 0, false,0,0};

volatile uint32_t g_enc_update_cycles = 0;
volatile uint32_t g_enc_update_us = 0;

/************************* 内部函数声明 *************************/
static void Encoder_Update(void);

/************************* 核心功能实现 *************************/
void BSP_Motor_Init(void)
{
    // 1. 驱动芯片使能、方向引脚初始化
    HAL_GPIO_WritePin(MOTOR_STBY_PORT, MOTOR_STBY_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(M1_IN1_PORT, M1_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M1_IN2_PORT, M1_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M2_IN1_PORT, M2_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M2_IN2_PORT, M2_IN2_PIN, GPIO_PIN_RESET);

    // 2. 启动 PWM
    HAL_TIM_PWM_Start(M1_PWM_TIM, M1_PWM_CH);
    HAL_TIM_PWM_Start(M2_PWM_TIM, M2_PWM_CH);

    // 3. 启动编码器
    __HAL_TIM_SET_COUNTER(M1_ENC_TIM, 0);
    __HAL_TIM_SET_COUNTER(M2_ENC_TIM, 0);
    HAL_TIM_Encoder_Start(M1_ENC_TIM, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(M2_ENC_TIM, TIM_CHANNEL_ALL);

    // 4. 状态初始化
    motor1.total_pulse = 0;
    motor1.speed_rpm = 0;
    motor1.last_pulse = (int32_t)__HAL_TIM_GET_COUNTER(M1_ENC_TIM);
    motor1.angle_deg = 0.0f;        // 【新增】初始化角度
    motor1.total_angle_deg = 0.0f;  // 【新增】初始化角度
    motor1.inited = true;

    motor2.total_pulse = 0;
    motor2.speed_rpm = 0;
    motor2.last_pulse = (int32_t)__HAL_TIM_GET_COUNTER(M2_ENC_TIM);
    motor2.angle_deg = 0.0f;        // 【新增】初始化角度
    motor2.total_angle_deg = 0.0f;  // 【新增】初始化角度
    motor2.inited = true;

    // 5. 启动 TIM6 硬件定时器
    HAL_TIM_Base_Start_IT(&htim6);
}

void BSP_Motor_SetSpeed1(int16_t speed)
{
    if (speed > PWM_MAX_SPEED) speed = PWM_MAX_SPEED;
    if (speed < -PWM_MAX_SPEED) speed = -PWM_MAX_SPEED;


   // DWT_Delay_us(2);
    if (speed >= 0) {
        HAL_GPIO_WritePin(M1_IN1_PORT, M1_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(M1_IN2_PORT, M1_IN2_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(M1_IN2_PORT, M1_IN2_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(M1_IN1_PORT, M1_IN1_PIN, GPIO_PIN_RESET);
    }

    uint16_t abs_speed = (speed < 0) ? -speed : speed;
    uint32_t pulse = (abs_speed * PWM_MAX_DUTY) / PWM_MAX_SPEED;
    __HAL_TIM_SET_COMPARE(M1_PWM_TIM, M1_PWM_CH, pulse);
}

void BSP_Motor_SetSpeed2(int16_t speed)
{
    if (speed > PWM_MAX_SPEED) speed = PWM_MAX_SPEED;
    if (speed < -PWM_MAX_SPEED) speed = -PWM_MAX_SPEED;

    if (speed >= 0) {
       HAL_GPIO_WritePin(M2_IN1_PORT, M2_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(M2_IN2_PORT, M2_IN2_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(M2_IN1_PORT, M2_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(M2_IN2_PORT, M2_IN2_PIN, GPIO_PIN_SET);
    }

    uint16_t abs_speed = (speed < 0) ? -speed : speed;
    uint32_t pulse = (abs_speed * PWM_MAX_DUTY) / PWM_MAX_SPEED;
    __HAL_TIM_SET_COMPARE(M2_PWM_TIM, M2_PWM_CH, pulse);
}

void BSP_Motor_Lock(void)
{
    taskENTER_CRITICAL();
}

void BSP_Motor_Unlock(void)
{
    taskEXIT_CRITICAL();
}

int16_t BSP_Motor_GetSpeed1(void)
{
    return motor1.speed_rpm;
}

int32_t BSP_Motor_GetPulse1(void)
{
    return motor1.total_pulse;
}
int16_t BSP_Motor_GetSpeed2(void)
{
    return motor2.speed_rpm;
}
int32_t BSP_Motor_GetPulse2(void)
{
    return motor2.total_pulse;
}
uint16_t BSP_Motor_GetBatteryVoltage(void)
{
    uint32_t adc_sum = 0;
    for (uint8_t i = 0; i < 5; i++) {
        HAL_ADC_Start(BAT_ADC_HANDLE);
        HAL_ADC_PollForConversion(BAT_ADC_HANDLE, 10);
        adc_sum += HAL_ADC_GetValue(BAT_ADC_HANDLE);
        HAL_ADC_Stop(BAT_ADC_HANDLE);
    }
    uint16_t adc_avg = adc_sum / 5;
    return (uint16_t)((adc_avg * 3300 / 4095) * BAT_VOLTAGE_SCALE);
}


float BSP_Motor_GetAngle1_Deg(void)
{
    float angle;
    taskENTER_CRITICAL();
    angle = motor1.angle_deg; // 直接读结构体
    taskEXIT_CRITICAL();
    return angle;
}

float BSP_Motor_GetTotalAngle1_Deg(void)
{
    float angle;
    taskENTER_CRITICAL();
    angle = motor1.total_angle_deg; // 直接读结构体
    taskEXIT_CRITICAL();
    return angle;
}

float BSP_Motor_GetAngle2_Deg(void)
{
    float angle;
    taskENTER_CRITICAL();
    angle = motor2.angle_deg;
    taskEXIT_CRITICAL();
    return angle;
}

float BSP_Motor_GetTotalAngle2_Deg(void)
{
    float angle;
    taskENTER_CRITICAL();
    angle = motor2.total_angle_deg;
    taskEXIT_CRITICAL();
    return angle;
}

// 【新增】辅助函数：封装角度计算逻辑
static void Update_Motor_Angle(Motor_State_t *motor)
{
    // 1. 计算累计总角度
    motor->total_angle_deg = (motor->total_pulse * 360.0f) / OUTPUT_PULSE_PER_ROUND;

    // 2. 计算单圈角度 [0, 360)
    int32_t remainder = motor->total_pulse % OUTPUT_PULSE_PER_ROUND;
    if (remainder < 0) remainder += OUTPUT_PULSE_PER_ROUND;
    motor->angle_deg = (remainder * 360.0f) / OUTPUT_PULSE_PER_ROUND;
}


static void Encoder_Update(void)
{
    uint32_t dwt_start_cycle = DWT_GetCycleCount();
    uint64_t dwt_start_us = DWT_GetMicroseconds();

    int32_t cur_pulse1 = (int32_t)__HAL_TIM_GET_COUNTER(M1_ENC_TIM);
    int32_t cur_pulse2 = (int32_t)__HAL_TIM_GET_COUNTER(M2_ENC_TIM);

    int32_t diff1 = cur_pulse1 - motor1.last_pulse;
    int32_t diff2 = cur_pulse2 - motor2.last_pulse;

    if (diff1 > 32767) diff1 -= 65536;
    else if (diff1 < -32768) diff1 += 65536;
    if (diff2 > 32767) diff2 -= 65536;
    else if (diff2 < -32768) diff2 += 65536;

    motor1.total_pulse += diff1;
    motor2.total_pulse += diff2;
    motor1.last_pulse = cur_pulse1;
    motor2.last_pulse = cur_pulse2;
    motor1.speed_rpm = (int16_t)(diff1 * RPM_FACTOR);
    motor2.speed_rpm = (int16_t)(diff2 * RPM_FACTOR);

    Update_Motor_Angle(&motor1);
    Update_Motor_Angle(&motor2);

    uint32_t dwt_end_cycle = DWT_GetCycleCount();
    uint64_t dwt_end_us = DWT_GetMicroseconds();
    g_enc_update_cycles = dwt_end_cycle - dwt_start_cycle;
    g_enc_update_us = (uint32_t)(dwt_end_us - dwt_start_us);
}

/* HAL 定时器回调：每 10ms 由 TIM6 中断触发 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        Encoder_Update();
    }
}