#include "../Inc/Balance_Task.h"
#include "bsp_motor.h"
#include "speed_pid.h"
#include "debug_vars.h"
#include <algorithm>

typedef struct {
    float target_speed;
    float target_angle;
    int16_t motor_pwm_out;
} BalanceControl_t;

int16_t angle_pid_calculate(float setpoint, float feedback, float gyro)
{
    // 核心思想：误差 = 目标 - 当前
    // 同时，D项（gyro）通常是减号，用于增加阻尼
    float angle_error = setpoint - feedback;

    // 请根据你的实际电机转向调整下面的符号 (+/-)
    // 如果车往一个方向倒，电机却往反方向转，就改这里的符号
    return (int16_t)( -116.8f * angle_error - 0.7f * gyro );
}

BalanceControl_t g_balance_ctrl = {0, 0, 0};
SpeedPID motor_speed_pid(0.1f,
    0.0025f, 0.0001f, 1000, 1000);

static uint8_t s_speed_loop_divider = 0;
static float s_encoder_speed_filtered = 0;

void BalanceTask::run()
{
    BSP_Motor_Lock();
    motor1.total_pulse = 0;
    motor2.total_pulse = 0;
    BSP_Motor_Unlock();
    speed_pid_clear();
    motor_speed_pid.Clear();

    for (;;) {
        float current_pitch = g_balance_pitch;
        float current_gyro = g_balance_gyro_y;

        int16_t m1_speed = BSP_Motor_GetSpeed1();
        int16_t m2_speed = BSP_Motor_GetSpeed2();
        float current_speed = (m1_speed + m2_speed) / 2.0f;

        s_speed_loop_divider++;
        if (s_speed_loop_divider >= 5) {
            s_speed_loop_divider = 0;

            s_encoder_speed_filtered = s_encoder_speed_filtered * 0.85f + current_speed * 0.15f;

            g_balance_ctrl.target_angle = motor_speed_pid.Calculate(
                g_balance_ctrl.target_speed,
                s_encoder_speed_filtered,
                0.005f
            );

            g_balance_ctrl.target_angle = std::clamp(g_balance_ctrl.target_angle, -15.0f, 15.0f);
         }

        int16_t angle_pid_out = angle_pid_calculate(g_balance_ctrl.target_angle, current_pitch, current_gyro);

        g_balance_ctrl.motor_pwm_out = angle_pid_out;
        g_balance_ctrl.motor_pwm_out = std::clamp(g_balance_ctrl.motor_pwm_out, (int16_t)-5000, (int16_t)5000);

        BSP_Motor_SetSpeed1(g_balance_ctrl.motor_pwm_out);
        BSP_Motor_SetSpeed2(g_balance_ctrl.motor_pwm_out);

        osDelay(1);
    }
}

extern "C" {
    static BalanceTask balanceTask;

    void BalanceTask_Init()
    {
        balanceTask.start((char*)"BalanceTask", 256, osPriorityHigh);
    }
}