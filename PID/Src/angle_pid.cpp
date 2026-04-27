//
// Created by lingard on 2026/1/10.
//

#include "../Inc/angle_pid.h"

#include <cmath>
#include <math.h>

void AnglePID::setTarget(int16_t target)
{
    target_angle = target;
}

int16_t AnglePID::calculate(float feedback, float dt)
{
    int32_t fb=handleZeroCross(target_angle,feedback);
    float output = Calculate(static_cast<float>(target_angle),static_cast<float>(fb),dt);
    return static_cast<int16_t>(output);
}

int32_t AnglePID::handleZeroCross(int32_t taget,int32_t feedback)
{
    int32_t error = taget - feedback;
    if(error>4096)
    {
        feedback += 8196;
    }else if(error<-4096)
    {
        feedback -= 8196;
    }
    return feedback;
}

extern "C"{
    static AnglePID angle_pid(2,0,0,5000,250);

    void angle_pid_clear()
    {
        angle_pid.Clear();
    }

    float angle_pid_caculate(float target,float feedback,float dt)
    {
        angle_pid.setTarget(target);
        return angle_pid.calculate(feedback,dt);
    }
}

