#include "../Inc/Balance_Task.h"
#include "bsp_motor.h"
#include "speed_pid.h"
#include "debug_vars.h"
#include "tcrt.h"
#include <algorithm>



int16_t angle_pid_calculate(float setpoint, float feedback, float gyro)
{
    // 核心思想：误差 = 目标 - 当前
    // 同时，D项（gyro）通常是减号，用于增加阻尼
    float angle_error = setpoint - feedback;

    // 请根据你的实际电机转向调整下面的符号 (+/-)
    // 如果车往一个方向倒，电机却往反方向转，就改这里的符号
    return (int16_t)(-158.55f * angle_error - 0.8f * gyro );
    //return (int16_t)(155.55f * angle_error + 0.8f * gyro );
}

BalanceControl_t g_balance_ctrl = {0, 0, 0};
 SpeedPID motor_speed_pid(0.05f,
                          0.0025f, 0.000001f, 1000, 1000); //0.008,0.000,0.000001
// SpeedPID steer_pid(0.8f, 0.05f, 0.0f, 300, 300);
// // 转向输出值
// float steer_out = 0;
// // 循迹传感器状态
 static uint8_t s_speed_loop_divider = 0;
static float s_encoder_speed_filtered = 0;
//uint8_t sensor_states[TCRT_CHANNEL_COUNT];

void BalanceTask::run()
{
    //TCRT_Init();

    BSP_Motor_Lock();
    motor1.total_pulse = 0;
    motor2.total_pulse = 0;
    BSP_Motor_Unlock();
    speed_pid_clear();
    motor_speed_pid.Clear();
    // g_balance_ctrl.target_speed = 30;

    for (;;) {
        float current_pitch = g_balance_pitch;
        //+0.298f;
        float current_gyro = g_balance_gyro_y;

        int16_t m1_speed = -BSP_Motor_GetSpeed1();
        int16_t m2_speed = -BSP_Motor_GetSpeed2();
        float current_speed = (m1_speed + m2_speed) / 2.0f;

        //TCRT_GetAllStates(sensor_states);

        //int8_t steer_error =

        // 仅【前进模式】开启转向，停止时转向归零
        // if (g_balance_ctrl.target_speed != 0) {
        //     // 转向PID计算
        //     steer_out = steer_pid.Calculate(0, steer_error, 0.005f);
        // } else {
        //     // 静止时无转向
        //     steer_out = 0;
        //     steer_pid.Clear();
        // }

        s_speed_loop_divider++;
        if (s_speed_loop_divider >= 5) {
            s_speed_loop_divider = 0;

            s_encoder_speed_filtered = s_encoder_speed_filtered * 0.91f + current_speed * 0.09f;

            g_balance_ctrl.target_angle = motor_speed_pid.Calculate(
                g_balance_ctrl.target_speed,
                s_encoder_speed_filtered,
                0.005f
            );

            g_balance_ctrl.target_angle = std::clamp(g_balance_ctrl.target_angle, -20.0f, 20.0f);
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
        balanceTask.start((char*)"BalanceTask", 512, osPriorityHigh);
    }
}