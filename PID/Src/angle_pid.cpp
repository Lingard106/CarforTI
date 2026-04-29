//
// Created by lingard on 2026/1/10.
//

#include "../Inc/angle_pid.h"

#include <cmath>
#include <math.h>
static float normalizeAngle(float angle) {
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}
void AnglePID::setTarget(int16_t target)
{
    target_angle = normalizeAngle(target);
}

int16_t AnglePID::calculate(float feedback, float dt)
{
    // int32_t fb=handleZeroCross(target_angle,feedback);
    // float output = Calculate(static_cast<float>(target_angle),static_cast<float>(fb),dt);
    // return static_cast<int16_t>(output);
    // 目标值提前规范化（setTarget 时已做）
    float target = target_angle;
    // 计算误差并规范化
    float error = target - feedback;
    // 将误差规范到 [-180, 180)，避免绕远路
    while (error > 180.0f)  error -= 360.0f;
    while (error < -180.0f) error += 360.0f;

    // 直接使用基类的 PID 计算，但传入误差和反馈（这里不再需要基类的 prevError 做差分，因为 D 项我们可以直接使用陀螺仪角速度，所以基类的 Kd*derivative 可保留或弃用）
    // 注意：基类 Calculate 会重新计算 error = target - actual，因此我们必须传入一个虚假的 target 和 actual，使得基类内部 error = 我们规范后的值。
    // 简单做法：直接调用基类的 Calculate 并传入 target = feedback + error，actual = feedback，则基类内部 error = (feedback+error) - feedback = error。
    float fakeTarget = feedback + error;
    float output = PidBase::Calculate(fakeTarget, feedback, dt);
    return output;
}

// int32_t AnglePID::handleZeroCross(int32_t taget,int32_t feedback)
// {
//     int32_t error = taget - feedback;
//     if(error>4096)
//     {
//         feedback += 8196;
//     }else if(error<-4096)
//     {
//         feedback -= 8196;
//     }
//     return feedback;
// }

extern "C"{
    static AnglePID angle_pid(200,0,0,5000,250);

    void angle_pid_clear()
    {
        angle_pid.Clear();
    }

    float angle_pid_calculate(float target,float feedback,float dt)
    {
        angle_pid.setTarget(target);
        return angle_pid.calculate(feedback,dt);
    }
}

